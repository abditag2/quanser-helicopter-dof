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

double P[10] = {-0.500, -2.4000, 0.00,  0.1200, 0.1200, -2.5000, -0.0200, 0.200, 2.1000, 10.0000};

double Hx[34][6] = {  

	{-0.948683,-0.316228,0.000000,0.000000,0.000000,0.000000},
//	{1.000000,0.000000,0.000000,0.000000,0.000000,0.000000},
	{0.000000,1.000000,0.000000,0.000000,0.000000,0.000000},
	{0.000000,0.000000,0.000000,1.000000,0.000000,0.000000},
	{0.000000,0.000000,0.000000,0.000000,1.000000,0.000000},
	{-0.948683,0.316228,0.000000,0.000000,0.000000,0.000000},
	{0.000000,-1.000000,0.000000,0.000000,0.000000,0.000000},
	{0.000000,0.000000,0.000000,-1.000000,0.000000,0.000000},
	{0.000000,0.000000,0.000000,0.000000,-1.000000,0.000000},
	{0.000000,-0.980581,0.000000,0.000000,-0.196116,0.000000},
	{-0.976164,-0.092968,0.000000,-0.195233,-0.018594,0.000000},
	{-0.948628,0.196634,0.000000,-0.245527,0.034012,0.000000},
	{0.896258,0.256074,0.000000,0.358503,0.051215,0.000000},
	{0.980030,0.031112,0.000000,0.196006,0.012445,0.000000},
	{0.976164,0.092968,0.000000,0.195233,0.018594,0.000000},
	{-0.948628,-0.196634,0.000000,-0.245527,-0.034012,0.000000},
	{-0.924294,-0.088028,0.000000,-0.369718,-0.035211,0.000000},
	{0.924294,-0.088028,0.000000,0.369718,-0.035211,0.000000},
	{-0.947201,0.209313,0.000000,-0.240390,0.034805,0.000000},
	{-0.931312,0.310437,0.000000,-0.186262,0.039913,0.000000},
	{0.000000,0.980581,0.000000,0.000000,0.196116,0.000000},
	{0.980030,-0.031112,0.000000,0.196006,-0.012445,0.000000},
	{0.980581,0.000000,0.000000,0.196116,0.000000,0.000000},
	{-0.930261,-0.310087,0.000000,-0.186052,-0.062017,0.000000},
	{-0.947201,-0.209313,0.000000,-0.240390,-0.034805,0.000000},
	{0.924294,0.088028,0.000000,0.369718,0.035211,0.000000},
	{-0.976164,0.092968,0.000000,-0.195233,0.018594,0.000000},
	{-0.930261,0.310087,0.000000,-0.186052,0.062017,0.000000},
	{-0.935982,0.231767,0.000000,-0.262075,0.039222,0.000000},
	{0.896258,-0.256074,0.000000,0.358503,-0.051215,0.000000},
	{0.976164,-0.092968,0.000000,0.195233,-0.018594,0.000000},
	{-0.935982,-0.231767,0.000000,-0.262075,-0.039222,0.000000},
	{-0.931312,-0.310437,0.000000,-0.186262,-0.039913,0.000000},
	{-0.924294,0.088028,0.000000,-0.369718,0.035211,0.000000},
};

double hx[34][1] ={
	{0.125968},
//	{0.219471},
	{0.932452},
	{0.199735},
	{1.300000},
	{0.125968},
	{0.932452},
	{0.3},
//	{0.199735},
	{1.300000},
	{1.120267},
	{0.149140},
	{0.161673},
	{0.507180},
	{0.270233},
	{0.320451},
	{0.161673},
	{0.196673},
	{0.358882},
	{0.162316},
	{0.171114},
	{1.120267},
	{0.270233},
	{0.234821},
	{0.188640},
	{0.162316},
	{0.358882},
	{0.149140},
	{0.188640},
	{0.172952},
	{0.507180},
	{0.320451},
	{0.172952},
	{0.171114},
	{0.196673},
};



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
	int safe;
} state;  

struct command{
	double u1;
	double u2;
} command;

double SIM_STEP = 0.01;
double PERIOD = 0.05;
double RESTART_TIME  = 0.4;
pthread_t tid;
int record =1;

double MAX_VOLTAGE = 3;


double vol_right = 0;
double vol_left = 0;


double add_left = 0;
double add_right = 0 ;

double C[34][1];
void matrix_mult(double A[34][6], double B[6][1], int m, int n, int k){

	int r = 0;
	int c = 0; 
	int kk = 0;
	for ( r = 0 ; r < m; r ++){
		for (c = 0; c < n; c ++ ){
			C[r][c] = 0 ;
			for ( kk = 0 ; kk < k ; kk ++){
				C[r][c] += A[r][kk] * B[kk][c] ;  
			}
		}
	}

}

double voltage_max_min(double voltage){

	if (voltage > MAX_VOLTAGE ) 
		voltage = MAX_VOLTAGE;
	else if (voltage < -MAX_VOLTAGE)
		voltage = -MAX_VOLTAGE;
	return voltage;
}




struct state eval_state(struct state state_x, struct command U){

	struct state d_state;

	d_state.elevation = state_x.d_elevation;
	d_state.pitch = state_x.d_pitch;
	d_state.travel = state_x.d_travel;
	d_state.d_elevation = P[0]*cos(state_x.elevation)+ P[1]*sin(state_x.elevation) + P[2]*state_x.d_travel	+ P[7]*cos(state_x.pitch)*(U.u1+U.u2) ; 
	d_state.d_pitch = P[4]*sin(state_x.pitch) + P[3]*cos(state_x.pitch)+ P[5]*state_x.d_pitch		+ P[8]*(U.u1-U.u2);
	d_state.d_travel = P[6]*state_x.d_travel								+ P[9]*sin(state_x.pitch)*(U.u1+U.u2);

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

	U.u1 =  0.8 + -6.5 * x.elevation  - .701 * x.pitch  -45.7161 * PERIOD * x.d_elevation -3.051 * PERIOD * x.d_pitch ; //-0.0333*cs->int_elevation -0.001*cs->int_pitch;
	U.u2  = 0.8 + -6.5 * x.elevation  + .5701 * x.pitch -45.7529 * PERIOD * x.d_elevation +5.970 * PERIOD*  x.d_pitch; //-0.03*cs->int_elevation +0.001*cs->int_pitch;



	cs->elevation2 = cs->elevation1;
	cs->elevation1 = x.elevation;

	cs->pitch2 = cs->pitch1;
	cs->pitch1 = x.pitch;

	cs->travel2 = cs->travel1;
	cs->travel1 = x.travel;	


	U.u1 = voltage_max_min(U.u1);
	U.u2 = voltage_max_min(U.u2);
	return U;
}

struct command controller_complex(struct state x, struct controller_storage* cs){
	struct command U;
	//printf("\n:info int_elevation: %lf elevation_conv: %lf int_pitch:%lf, pitch_conv:%lf\n", int_elevation, elevation_conv, int_pitch, pitch_conv);

	cs->int_travel +=  x.travel;
	cs->int_pitch +=  x.pitch;
	cs->int_elevation +=  x.elevation;


	//	U.u1 = 1+ -6.5 * x.elevation  - 3.01 * (x.pitch + 0.08)  -25.7161 * x.d_elevation ;//-3.051 * x.d_pitch; //-.0333*cs->int_elevation -0.001*cs->int_pitch;
	//	U.u2  = 1 + -6.5 * x.elevation  + 5.5701 * (x.pitch + 0.08) -25.7529 * x.d_elevation; // +5.970 * x.d_pitch; //-0.03*cs->int_elevation +0.001*cs->int_pitch;

	U.u1 =   -1.5 * x.elevation  - .701 * (x.pitch-0.2)  -45.7161 * PERIOD * x.d_elevation -3.051 * PERIOD * x.d_pitch + 0.2 * x.travel; //-0.0333*cs->int_elevation -0.001*cs->int_pitch;
	U.u2  =  -1.5 * x.elevation  + .5701 * (x.pitch-0.2)  -45.7529 * PERIOD * x.d_elevation +5.970 * PERIOD * x.d_pitch - 0.2 * x.travel; //-0.03*cs->int_elevation +0.001*cs->int_pitch;



	cs->elevation2 = cs->elevation1;
	cs->elevation1 = x.elevation;

	cs->pitch2 = cs->pitch1;
	cs->pitch1 = x.pitch;

	cs->travel2 = cs->travel1;
	cs->travel1 = x.travel;	

	U.u1 += add_right;
	U.u2 += add_left;


	U.u1 = voltage_max_min(U.u1);
	U.u2 = voltage_max_min(U.u2);

	return U;
}


int check_safety(struct state x){

	double X [6][1] = {{x.elevation}, {x.pitch}, {x.travel}, {x.d_elevation}, {x.d_pitch}, {x.d_travel}};

	matrix_mult(Hx, X, 34, 1, 6);

	int all_small = 1;
	int k = 0;
	for (k = 0 ; k < 34 ; k++){
		if(C[k][0] > hx[k][0]){

			all_small = 0;
			int rr = 0 ;
			printf("\n\n\n");

				printf("%d\n", k);
			for ( rr = 0 ; rr < 34; rr++){
				printf ("%d \t  hx: %f rr: %lf\n", rr, hx[rr][0], C[rr][0]);
			}


			break;
		}

	}

	//	if (x.elevation+0.333*x.pitch > -0.3 && x.elevation-0.333*x.pitch>-0.3  && x.elevation < 0.35 && x.d_elevation > -0.3 && x.d_elevation < 0.4  && x.d_pitch > -1.3 && x.d_pitch < 1.3)
	if(all_small == 1)
	{
		return 1;
	}
	else
		return 0;
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
		if(check_safety(state_x) == 0)
		{
			state_x.safe = 0;
			printf("sim fina1: elevation: %lf, pitch: %lf, travel: %lf, d_elevation: %lf, d_pitch: %lf, d_travel: %lf\n", state_x.elevation, state_x.pitch, state_x.travel, state_x.d_elevation, state_x.d_pitch, state_x.d_travel);


			return state_x;
		}
	}
	state_x.safe = 1;

	printf("sim fina: elevation: %lf, pitch: %lf, travel: %lf, d_elevation: %lf, d_pitch: %lf, d_travel: %lf\n", state_x.elevation, state_x.pitch, state_x.travel, state_x.d_elevation, state_x.d_pitch, state_x.d_travel);

	return state_x;
}

struct state simulate_with_controller(struct state init_state, double time){

	struct state state_x;

	state_x = init_state;

	int steps = time/SIM_STEP;
	struct controller_storage cs;
	cs.int_elevation = 0;
	cs.int_pitch = 0;
	cs.int_travel = 0;

	int k = 0;
	for (k = 0; k <steps; k++){
		struct command U = controller_safety(state_x, &cs);
		state_x = eval_state(state_x, U);
		if ( check_safety(state_x) == 0){

			state_x.safe = 0;
			printf("sim fina2: elevation: %lf, pitch: %lf, travel: %lf, d_elevation: %lf, d_pitch: %lf, d_travel: %lf\n", state_x.elevation, state_x.pitch, state_x.travel, state_x.d_elevation, state_x.d_pitch, state_x.d_travel);


			return state_x;
		}
	}

	state_x.safe = 1;
	printf("sim fina: elevation: %lf, pitch: %lf, travel: %lf, d_elevation: %lf, d_pitch: %lf, d_travel: %lf\n", state_x.elevation, state_x.pitch, state_x.travel, state_x.d_elevation, state_x.d_pitch, state_x.d_travel);

	return state_x;
}




//The outcome determines weather the safety controller should be used or not
int decide(struct state current_state, struct command U, double time){
	//struct state x2 = simulate_fixed_control(current_state, U, time);
	struct state x2 = simulate_fixed_control(current_state, U, RESTART_TIME);
	struct state x10 = simulate_with_controller(x2, 1);
	printf ("sim results: elev: %lf %d %d\n", x2.elevation, x2.safe, x10.safe);

	if (x2.safe == 1 && x10.safe == 1)
		return 1;
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
			add_left = add_left - 0.05;
		}
		else if (in == '2'){
			add_left = add_left + 0.05;
			//record = 0;
			//printf("recording ended\n");
		}
		else if (in == '='){	
			add_right = add_right + 0.01;
		}
		else if (in == '-'){
			add_right = add_right - 0.01;
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

	int remaining_safety_cycles = 0 ;
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

		struct state cs;

		cs.elevation = 1.0/10.0 * 3.1415/180.0 * (double) elevation;
		cs.pitch =  90.0/1000.0 * 3.1415/180.0 * (double) pitch;
		cs.travel = 3.1415/4089.00 * (double) travel;

		cs.d_travel =  (cs.travel - storage.travel1)/PERIOD;
		cs.d_pitch  = (cs.pitch - storage.pitch1)/PERIOD;
		cs.d_elevation = (cs.elevation - storage.elevation1)/PERIOD;			

		storage.elevation2 = storage.elevation1;
		storage.elevation1 = cs.elevation;

		storage.pitch2 = storage.pitch1;
		storage.pitch1 = cs.pitch;

		storage.travel2 = storage.travel1;
		storage.travel1 = cs.travel;			


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
			struct command U_safety = controller_safety(cs, &storage_safety);
			struct command U_complex = controller_complex(cs, &storage_complex);		
			printf("remaining_cycle %d\n", remaining_safety_cycles);



			if (decide(cs, U_complex, 0.2) == 1 && (remaining_safety_cycles <= 0 )  ) {
				printf("complex controller\n");
				vol_right = U_complex.u1;
				vol_left = U_complex.u2;
			}
			else {
				printf("safety controller\n");
				vol_right = U_safety.u1;
				vol_left = U_safety.u2;
				remaining_safety_cycles -= 1;
			}
		}


		if(step % 200 == 0){

			printf("restart\n");
			usleep(RESTART_TIME *1000000.0);
			remaining_safety_cycles = 60;

		}


		//		vol_right = 0;
		//		vol_left = 0;
		//		printf ("elev -1/3* pitch : %lf elev + 1.0/3.0*pitch: %lf\n", cs.elevation-0.3333*cs.pitch, cs.elevation + 0.3333*cs.pitch);

		if (vol_right > MAX_VOLTAGE ) 
			vol_right = MAX_VOLTAGE;
		else if (vol_right < -MAX_VOLTAGE)
			vol_right = -MAX_VOLTAGE;

		if (vol_left > MAX_VOLTAGE) 
			vol_left = MAX_VOLTAGE;
		else if (vol_left < -MAX_VOLTAGE)
			vol_left = -MAX_VOLTAGE;


		vol_right = voltage_max_min(vol_right);
		vol_left = voltage_max_min(vol_left);
		usleep ((int) (PERIOD * 1000000.0));
	}
	fclose(ofp);	
	return 0;
}



