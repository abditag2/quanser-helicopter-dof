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

double SIM_STEP = 0.01;

struct state simulate(struct state init_state, double u1, double u2, double time){

	struct state d_state;
	struct state state_x;

	state_x = init_state;

	int steps = time/SIM_STEP;
	int k = 0;
	for (k = 0; k <steps; k++){

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

	}

	return state_x;
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

	int johny[3];

	err = ioctl(File_Descriptor, Q8_ENC, johny);
	if(err != 0)
	{
		perror("Epic Fail first enc read\n");
		return -1;
	}

	int base_travel = johny[0];
	int base_pitch = johny[1];
	int base_elevation = johny[2];


	float elevation1 = 0;
	float elevation2 = 0;

	float pitch1 = 0;
	float pitch2 = 0;

	float travel1 = 0;
	float travel2 = 0;

	float int_elevation = 0;
	float int_pitch = 0;
	float int_travel = 0;	

	float d_travel = 0;
	float d_pitch = 0;
	float d_elevation = 0;

	for (step = 0 ; step < 15000 ; step++){

		unsigned short int tmparray[4];
		tmparray[0] = Q8_dacVTO((vol_right), 1, 10);
		tmparray[1] = Q8_dacVTO((vol_left), 1, 10);
		ioctl(File_Descriptor, Q8_WR_DAC, tmparray);


		/* .. Reading the Encoder values from the helicopter......*/

		err = ioctl(File_Descriptor, Q8_ENC, johny);
		if(err != 0)
		{
			perror("Epic Fail first enc read\n");
			return -1;
		}

		int travel = johny[0] - base_travel;
		int pitch = johny[1] - base_pitch;
		int elevation = -(johny[2] - base_elevation)-300;


		printf("\n step: %d, travel = %d, pitch = %d, elevation = %d, left: %lf, right: %lf\n", step, travel, pitch , elevation, vol_left, vol_right); 

		if (record == 1){
			fprintf(ofp, "%d\t%d\t%d\t%d\t%lf\t%lf\n", step, travel, pitch, elevation, vol_right, vol_left);
		}

		float elevation_conv = 1.0/10.0 * 3.1415/180.0 * (float) elevation;
		float pitch_conv =  90.0/1000.0 * 3.1415/180.0 * (float) pitch;
		float travel_conv = (float) travel;

		if (step < 50) {
			vol_right = 0;
			vol_left = 0;
		}
		else{
			// elevation * 1/10 * 3.1415/180
			// pitch * 90/1000 * 3.1415/180

			printf("\n:info int_elevation: %lf elevation_conv: %lf int_pitch:%lf, pitch_conv:%lf\n", int_elevation, elevation_conv, int_pitch, pitch_conv);

			int_travel +=  travel_conv;
			int_pitch +=  pitch_conv;
			int_elevation +=  elevation_conv;

			d_travel =  (travel_conv - travel1);
			d_pitch  = (pitch_conv - pitch1);
			d_elevation = (elevation_conv - elevation1);			


			vol_right = -4.5 * elevation_conv  - .701 * pitch_conv  -25.7161 * d_elevation -3.051 * d_pitch -0.0333*int_elevation -0.001*int_pitch;
			vol_left  = -4.5 * elevation_conv  + .5701 * pitch_conv -25.7529 * d_elevation +5.970 * d_pitch -0.03*int_elevation +0.001*int_pitch;



		}

		elevation2 = elevation1;
		elevation1 = elevation_conv;

		pitch2 = pitch1;
		pitch1 = pitch_conv;

		travel2 = travel1;
		travel1 = travel_conv;	

		//else if(step < 140){ 
		//	vol_right = 1.2;
		//	vol_left = 1.235;			
		//}
		/*
		   else if (step < 100) {
		   vol_right = 1.26;
		   vol_left =  1.285;
		   }
		   else if (step < 130){
		   vol_right = 1.38;
		   vol_left =  1.40;
		   }

		   else if ( step < 160){
		//			vol_right = 1.50;
		//			vol_left = 1.48;
		}

		 */		

		if (vol_right > MAX_VOLTAGE ) 
			vol_right = MAX_VOLTAGE;
		else if (vol_right < -MAX_VOLTAGE)
			vol_right = -MAX_VOLTAGE;

		if (vol_left > MAX_VOLTAGE) 
			vol_left = MAX_VOLTAGE;
		else if (vol_left < -MAX_VOLTAGE)
			vol_left = -MAX_VOLTAGE;


		//vol_right = 0;
		//vol_left = 0;

		usleep (50000);
	}
	fclose(ofp);	
	return 0;
}



