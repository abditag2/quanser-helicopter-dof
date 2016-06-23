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


struct controller_storage{
	double int_elevation;
	double int_pitch;
	double int_travel;

	double elevation1;
	double pitch1;
	double travel1;

	double elevation2;
	double pitch2;
	double travel2;
}controller_storage;



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
pthread_t tid;
int record =1;

double MAX_VOLTAGE = 2.5;


double vol_right = 0;
double vol_left = 0;


struct state eval_state(struct state state_x, struct command U){

	struct state d_state;

	d_state.elevation = state_x.d_elevation;
	d_state.pitch = state_x.d_pitch;
	d_state.travel = state_x.d_travel;
	d_state.d_elevation = P[1]*cos(state_x.elevation)+ P[2]*sin(state_x.elevation) 			+ P[8]*cos(state_x.pitch)*(U.u1+U.u2) ; 
	d_state.d_pitch = P[5]*sin(state_x.pitch) + P[4]*cos(state_x.pitch)+ P[6]*state_x.d_pitch	+ P[9]*(U.u1-U.u2);
	d_state.d_travel = P[7]*state_x.d_travel							+ P[10]*sin(state_x.pitch)*(U.u1+U.u2);

	state_x.elevation += SIM_STEP*d_state.elevation;
	state_x.pitch += SIM_STEP*d_state.pitch;
	state_x.travel += SIM_STEP*d_state.travel;
	state_x.d_elevation += SIM_STEP*d_state.d_elevation;
	state_x.d_pitch += SIM_STEP*d_state.d_pitch;
	state_x.d_travel += SIM_STEP*d_state.d_travel;

	return state_x;

}
struct command controller_safety(struct state x, struct controller_storage* cs){

	struct command U;

	//printf("\n:info int_elevation: %lf elevation_conv: %lf int_pitch:%lf, pitch_conv:%lf\n", int_elevation, elevation_conv, int_pitch, pitch_conv);

	cs->int_travel +=  x.travel;
	cs->int_pitch +=  x.pitch;
	cs->int_elevation +=  x.elevation;

	double d_travel =  (x.travel - cs->travel1);
	double d_pitch  = (x.pitch - cs->pitch1);
	double d_elevation = (x.elevation - cs->elevation1);			

	U.u1 = -4.5 * x.elevation  - .701 * x.pitch  -25.7161 * d_elevation -3.051 * d_pitch -0.0333*cs->int_elevation -0.001*cs->int_pitch;
	U.u2  = -4.5 * x.elevation  + .5701 * x.pitch -25.7529 * d_elevation +5.970 * d_pitch -0.03*cs->int_elevation +0.001*cs->int_pitch;

	cs->elevation2 = cs->elevation1;
	cs->elevation1 = x.elevation;

	cs->pitch2 = cs->pitch1;
	cs->pitch1 = x.pitch;

	cs->travel2 = cs->travel1;
	cs->travel1 = x.travel;	

	return U;
}

struct command controller_complex(struct state x, struct controller_storage* cs){
	struct command U;
	U.u1 = 0; //MAX_VOLTAGE;
	U.u2 = 0; //MAX_VOLTAGE;
	return U;
}

struct state simulate_fixed_control(struct state init_state, struct command U, double time){

	struct state state_x;

	state_x = init_state;
	printf("comm u1:%lf u2:%lf\n", U.u1, U.u2);
	printf("sim init: elevation: %lf, pitch: %lf, travel: %lf, d_elevation: %lf, d_pitch: %lf, d_travel: %lf\n", init_state.elevation, init_state.pitch, init_state.travel, init_state.d_elevation, init_state.d_pitch, init_state.d_travel);

	int steps = time/SIM_STEP;
	int k = 0;
	for (k = 0; k <steps; k++){
		state_x = eval_state(state_x, U);
	}

	printf("sim fina: elevation: %lf, pitch: %lf, travel: %lf, d_elevation: %lf, d_pitch: %lf, d_travel: %lf\n", state_x.elevation, state_x.pitch, state_x.travel, state_x.d_elevation, state_x.d_pitch, state_x.d_travel);

	return state_x;
}


//The outcome determines weather the safety controller should be used or not
int decide(struct state current_state, struct command U){
	//simulate the system from current state for 2 seconds
	struct state x2 = simulate_fixed_control(current_state, U, 0.4);
	printf ("sim results: elev: %lf\n", x2.elevation);
	if (x2.elevation > -0.2 && x2.elevation < 0.2 && x2.d_elevation > -1 && x2.d_elevation < 1 && x2.pitch > -0.2 && x2.pitch < 0.2 && x2.d_pitch > -0.2 && x2.d_pitch < 0.2)
	{
		printf("decided 1\n");
		return 1;
	}
	else
		return 0;
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

	struct controller_storage storage_safety;
	storage_safety.int_travel = 0;
	storage_safety.int_pitch = 0;
	storage_safety.int_elevation = 0;

	struct controller_storage storage_complex;
	storage_complex.int_travel = 0;
	storage_complex.int_pitch = 0;
	storage_complex.int_elevation = 0;

	struct controller_storage storage; //for the current loop

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
			struct state cs;

			cs.elevation = 1.0/10.0 * 3.1415/180.0 * (double) elevation;
			cs.pitch =  90.0/1000.0 * 3.1415/180.0 * (double) pitch;
			cs.travel = (double) travel;

			cs.d_travel =  (cs.travel - storage.travel2)/0.1;
			cs.d_pitch  = (cs.pitch - storage.pitch2)/0.1;
			cs.d_elevation = (cs.elevation - storage.elevation2)/0.1;			

			storage.elevation2 = storage.elevation1;
			storage.elevation1 = cs.elevation;

			storage.pitch2 = storage.pitch1;
			storage.pitch1 = cs.pitch;

			storage.travel2 = storage.travel1;
			storage.travel1 = cs.travel;			

			struct command U_safety = controller_safety(cs, &storage_safety);
			struct command U_complex = controller_complex(cs, &storage_complex);		

			if (decide(cs, U_complex) == 1){
				//			if(0 == 1) {
				printf("complex controller\n");
				vol_right = U_complex.u1;
				vol_left = U_complex.u2;

			}
			else {
				printf("safety controller\n");
				vol_right = U_safety.u1;
				vol_left = U_safety.u2;
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



