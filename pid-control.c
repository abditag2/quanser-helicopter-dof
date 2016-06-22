#include "q8user-betti.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <math.h>


/*  #define DRIVER_NAME "./test"*/
#define Q8_DAC_RESOLUTION  (12)
#define Q8_DAC_ZERO        (0x0800)
#define DEBUG 0

double P[10] = {-1.000, -2.4000, -0.0943, 0.1200, 0.1200, -2.5000, -0.0200, 0.0600, 2.1000, 10.0000};

struct state {
	double elevation;
	double pitch;
	double travel;
	double d_elevation; 
	double d_pitch;
	double d_travel;
} state;  

struct command{
	double u1;
	double u2;
} command;

double SIM_STEP = 0.01;

struct state eval_state(struct state state_x, u1, u2){

	struct state d_state;

	d_state.elevation = state_x.d_elevation;
	d_state.pitch = state_x.d_pitch;
	d_state.travel = state_x.d_travel;
	d_state.d_elevation = P[1]*cos(state_x.elevation)+ P[2]*sin(state_x.elevation) 			+ P[8]*cos(state_x.pitch)*(u1+u2) ; 
	d_state.d_pitch = P[5]*sin(state_x.pitch) + P[4]*cos(state_x.pitch)+ P[6]*state_x.d_pitch	+ P[9]*(u1-u2);
	d_state.d_travel = P[7]*state_x.d_travel							+ P[10]*sin(state_x.pitch)*(u1+u2);

	state_x.elevation += SIM_STEP*d_state.elevation;
	state_x.pitch += SIM_STEP*d_state.pitch;
	state_x.travel += SIM_STEP*d_state.travel;
	state_x.d_elevation += SIM_STEP*d_state.d_elevation;
	state_x.d_pitch += SIM_STEP*d_state.d_pitch;
	state_x.d_travel += SIM_STEP*d_state.d_travel;

	return state_x;

}

double elevation1 = 0;
double elevation2 = 0;

double pitch1 = 0;
double pitch2 = 0;

double travel1 = 0;
double travel2 = 0;

double int_elevation = 0;
double int_pitch = 0;
double int_travel = 0;	




struct command controller(double elevation_conv, double pitch_conv, double travel_conv){

	struct command U;

	//printf("\n:info int_elevation: %lf elevation_conv: %lf int_pitch:%lf, pitch_conv:%lf\n", int_elevation, elevation_conv, int_pitch, pitch_conv);

	int_travel +=  travel_conv;
	int_pitch +=  pitch_conv;
	int_elevation +=  elevation_conv;

	d_travel =  (travel_conv - travel1);
	d_pitch  = (pitch_conv - pitch1);
	d_elevation = (elevation_conv - elevation1);			

	U.u1 = -4.5 * elevation_conv  - .701 * pitch_conv  -25.7161 * d_elevation -3.051 * d_pitch -0.0333*int_elevation -0.001*int_pitch;
	U.u2  = -4.5 * elevation_conv  + .5701 * pitch_conv -25.7529 * d_elevation +5.970 * d_pitch -0.03*int_elevation +0.001*int_pitch;

	elevation2 = elevation1;
	elevation1 = elevation_conv;

	pitch2 = pitch1;
	pitch1 = pitch_conv;

	travel2 = travel1;
	travel1 = travel_conv;	

	return U;
}


struct state simulate_fixed_control(struct state init_state, double u1, double u2, double time){

	struct state state_x;

	state_x = init_state;

	int steps = time/SIM_STEP;
	int k = 0;
	for (k = 0; k <steps; k++){
		state_x = eval_state(state_x, u1, u2);
	}

	return state_x;
}

struct state simulate_with_control(struct state init_state, double time){

	struct state state_x;

	state_x = init_state;

	int steps = time/SIM_STEP;
	int k = 0;
	for (k = 0; k <steps; k++){
		struct command U = controller(double elevation, double pitch, double travel);
		state_x = eval_state(state_x, U.u1, U.u2);
	}

	return state_x;
}

//The outcome determines weather the safety controller should be used or not
bool decide(struct state current_state, struct command U){
	//simulate the system from current state for 2 seconds
	struct state x2 = simulate_fixed_control(current_state, U.u1, U.u2, 2);
	if (x2.elevation > 0.-1 && x2.elevation < 0.1 && x2.pitch > -0.2 && x2.pitch < 0.2 && x2.d_elevation > -0.2 && x2.d_elevation < 0.2 && x2.d_pitch > -0.2 && x2.d_pitch < 0.2)
		return true;
	else
		return false;
}



unsigned short int Q8_dacVTO( double nVoltage, int bBipolar, double nRange )
{
	if (bBipolar)
	{
		const double nOneLSB = 2 * nRange / (1 << Q8_DAC_RESOLUTION); /* volts per LSB*/
		if (DEBUG)
		{
			printf("DEBUG: nOneLSB = %f\n", nOneLSB);
			printf("DEBUG: con1 = %f \n", (-nRange+nOneLSB/2));
			printf("DEBUG: con2 = %f \n", (nRange - 1.5*nOneLSB));
			printf("DEBUG: valu = %d \n", ((unsigned short int)(nVoltage/nOneLSB)+Q8_DAC_ZERO));
		}
		if (nVoltage < -nRange + nOneLSB/2)
			return 0;
		else if (nVoltage >= nRange - 1.5*nOneLSB)
			return 0x0fffL;
		else
			return ((unsigned short int)(nVoltage / nOneLSB) + Q8_DAC_ZERO);
	}
	else
	{
		const double nOneLSB = nRange / (1 << Q8_DAC_RESOLUTION); /* volts per LSB */
		if (nVoltage < nOneLSB/2)
			return 0;
		else if (nVoltage >= nRange - 1.5*nOneLSB)
			return 0x0fff;
		else
			return (unsigned short int)(nVoltage / nOneLSB);
	}
}

pthread_t tid;
int record =1;

double MAX_VOLTAGE = 2.5;


double vol_right = 0;
double vol_left = 0;


void* get_keyboard(void *arg){

	while(1){
		char in;
		scanf("%c", &in);
		if (in == '1'){
			//printf("recording started\n");
			//record = 1;i
			vol_left = vol_left - 0.01;
		}
		else if (in == '2'){
			vol_left = vol_left + 0.01;
			//record = 0;
			//printf("recording ended\n");
		}
		else if (in == '='){	
			vol_right = vol_right + 0.01;
		}
		else if (in == '-'){
			vol_right = vol_right - 0.01;
		}
		usleep(30000);
	}
}



int main()
{
	int File_Descriptor, i, j; /**/
	int Return_Value ; /*To catch the return value .......*/
	char Buffer[Q8_EXAMPLE2_MAXLEN] ;
	int tmp;

	/* .................Opening the device (Read/Write)................*/


	int err = pthread_create(&tid, NULL, &get_keyboard, NULL);
	if (err != 0)
		printf("second thread not created\n");
	else
		printf("thread created\n"); 

	printf("\n\tOpening the device ..please wait...\n");
	fflush(stdout);

	File_Descriptor = open(DEVICE_FILE_NAME, O_RDWR);

	if( File_Descriptor < 0 ) /*Display a message if any error occures*/
	{
		perror("open");
		fprintf( stderr, "\n\tCould not open device file - Error : %i\n", File_Descriptor );
		return -1;
	}

	//ioctl(File_Descriptor, Q8_IOCTL_EXAMPLE1);
	//ioctl(File_Descriiptor, Q8_IOCTL_EXAMPLE2, Buffer);
	//for (i = 0; i < Q8_EXAMPLE2_MAXLEN; i ++)
	//printf ("%c", Buffer[i]);
	//printf ("\n");
	//ioctl(File_Descriptor, Q8_IOCTL_EXAMPLE3, 5);
	//ioctl(File_Descriptor, Q8_RD_ENC1, &tmp);
	//printf("ENC1 = %x \n", tmp);

	/* .. Writing the control values to the left and write motor......*/

	int step = 0;
	double vol_step = 0.1; 	

	FILE *ofp;
	ofp = fopen("recorded_data.txt", "w");

	int sensor_readings[3];

	err = ioctl(File_Descriptor, Q8_ENC, sensor_readings);
	if(err != 0)
	{
		perror("Epic Fail first enc read\n");
		return -1;
	}

	int base_travel = sensor_readings[0];
	int base_pitch = sensor_readings[1];
	int base_elevation = sensor_readings[2];


	double d_travel = 0;
	double d_pitch = 0;
	double d_elevation = 0;

	for (step = 0 ; step < 15000 ; step++){

		unsigned short int tmparray[4];
		tmparray[0] = Q8_dacVTO((vol_right), 1, 10);
		tmparray[1] = Q8_dacVTO((vol_left), 1, 10);
		ioctl(File_Descriptor, Q8_WR_DAC, tmparray);


		/* .. Reading the Encoder values from the helicopter......*/

		err = ioctl(File_Descriptor, Q8_ENC, sensor_readings);
		if(err != 0)
		{
			perror("Epic Fail first enc read\n");
			return -1;
		}

		int travel = sensor_readings[0] - base_travel;
		int pitch = sensor_readings[1] - base_pitch;
		int elevation = -(sensor_readings[2] - base_elevation)-300;

		printf("\n step: %d, travel = %d, pitch = %d, elevation = %d, left: %lf, right: %lf\n", step, travel, pitch , elevation, vol_left, vol_right); 

		if (record == 1){
			fprintf(ofp, "%d\t%d\t%d\t%d\t%lf\t%lf\n", step, travel, pitch, elevation, vol_right, vol_left);
		}

		if (step < 50 ){
			vol_right = 0;
			vol_left = 0;
		}
		else 
		{
			double elevation_conv = 1.0/10.0 * 3.1415/180.0 * (double) elevation;
			double pitch_conv =  90.0/1000.0 * 3.1415/180.0 * (double) pitch;
			double travel_conv = (double) travel;

			if (decide(current_state, U)){

			}
			else {
				struct U = controller(elevation_conv, pitch_conv, travel_conv);
				vol_right = U.u1;
				vol_left = U.u2;
			}
		}

		if (vol_right > MAX_VOLTAGE ) 
			vol_right = MAX_VOLTAGE;
		else if (vol_right < -MAX_VOLTAGE)
			vol_right = -MAX_VOLTAGE;

		if (vol_left > MAX_VOLTAGE) 
			vol_left = MAX_VOLTAGE;
		else if (vol_left < -MAX_VOLTAGE)
			vol_left = -MAX_VOLTAGE;

		usleep (50000);
	}
	fclose(ofp);	
	return 0;
}



