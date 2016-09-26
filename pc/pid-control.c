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
#define HX_SIZE 52


#define SIM_STEP  0.01
#define PERIOD  0.05
#define RESTART_TIME 0.3



double P[10] = {-.500, -2.4000, 0.00,  0.1200, 0.1200, -2.5000, -0.0200, 0.200, 2.1000, 10.0000};

double Hx[HX_SIZE][6] = {  
{-0.937749,-0.347314,0,0.000000,0.000000,0},
{1.000000,0.000000,0,0.000000,0.000000,0},
{0.000000,0.000000,0,1.000000,0.000000,0},
{0.000000,0.000000,0,0.000000,1.000000,0},
{-0.937749,0.347314,0,0.000000,0.000000,0},
{-1.000000,0.000000,0,0.000000,0.000000,0},
{0.000000,0.000000,0,-1.000000,0.000000,0},
{0.000000,0.000000,0,0.000000,-1.000000,0},
{-0.920831,0.341049,0,-0.184166,0.042875,0},
{0.980581,0.000000,0,0.196116,0.000000,0},
{0.976164,0.092968,0,0.195233,0.018594,0},
{0.976164,-0.092968,0,0.195233,-0.018594,0},
{0.982846,0.026635,0,0.182186,0.010654,0},
{0.983313,0.033791,0,0.178416,0.011006,0},
{-0.980581,0.000000,0,-0.196116,0.000000,0},
{-0.942434,-0.177818,0,-0.280952,-0.035564,0},
{-0.920831,-0.341049,0,-0.184166,-0.042875,0},
{-0.939588,0.229568,0,-0.251069,0.037920,0},
{-0.929818,0.247341,0,-0.269327,0.041529,0},
{-0.945071,0.207015,0,-0.250444,0.035553,0},
{0.730320,-0.429600,0,0.524112,-0.085920,0},
{0.730320,0.429600,0,0.524112,0.085920,0},
{0.983313,-0.033791,0,0.178416,-0.011006,0},
{0.982846,-0.026635,0,0.182186,-0.010654,0},
{-0.919538,-0.340570,0,-0.183908,-0.068114,0},
{-0.929818,-0.247341,0,-0.269327,-0.041529,0},
{-0.945071,-0.207015,0,-0.250444,-0.035553,0},
{-0.924294,0.088028,0,-0.369718,0.035211,0},
{-0.942434,0.177818,0,-0.280952,0.035564,0},
{-0.976164,0.092968,0,-0.195233,0.018594,0},
{-0.942675,0.223627,0,-0.244964,0.036715,0},
{-0.919538,0.340570,0,-0.183908,0.068114,0},
{-0.942675,-0.223627,0,-0.244964,-0.036715,0},
{-0.939588,-0.229568,0,-0.251069,-0.037920,0},
{-0.976164,-0.092968,0,-0.195233,-0.018594,0},
{-0.924294,-0.088028,0,-0.369718,-0.035211,0},
{0.924294,0.088028,0,0.369718,0.035211,0},
{0.924294,-0.088028,0,0.369718,-0.035211,0},

};



double hx[HX_SIZE][1] ={
{0.114896},
{0.119671},
{0.199735},
{1.300000},
{0.114896},
{0.119671},
{0.199735},
{1.300000},
{0.164176},
{0.136959},
{0.197136},
{0.197136},
{0.157410},
{0.160876},
{0.136959},
{0.160740},
{0.164176},
{0.156719},
{0.165424},
{0.152473},
{0.496346},
{0.496346},
{0.160876},
{0.157410},
{0.184185},
{0.165424},
{0.152473},
{0.185233},
{0.160740},
{0.137058},
{0.153904},
{0.184185},
{0.153904},
{0.156719},
{0.137058},
{0.185233},
{0.242119},
{0.242119},

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

pthread_t tid;
int record =1;

double MAX_VOLTAGE = 3;;


double vol_right = 0;
double vol_left = 0;


double add_left = 0;
double add_right = 0 ;

double C[HX_SIZE][1];

struct state sps;
void matrix_mult(double A[HX_SIZE][6], double B[6][1], int m, int n, int k){

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
struct command controller_safety(struct state sp, struct state x, struct controller_storage* cs){

	struct command U;

	cs->int_travel +=  x.travel;
	cs->int_pitch +=  x.pitch;
	cs->int_elevation +=  x.elevation;

	U.u1 = -6.5 * (x.elevation-sp.elevation) - .701 * x.pitch  - 45.7161 * PERIOD * x.d_elevation -3.051 * PERIOD * x.d_pitch ; //-0.0333*cs->int_elevation -0.001*cs->int_pitch;
	U.u2 = -6.5 * (x.elevation-sp.elevation) + .5701 * x.pitch - 45.7529 * PERIOD * x.d_elevation +5.970 * PERIOD*  x.d_pitch; //-0.03*cs->int_elevation +0.001*cs->int_pitch;

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

struct command controller_complex(struct state sp, struct state x, struct controller_storage* cs){
	struct command U;
	//printf("\n:info int_elevation: %lf elevation_conv: %lf int_pitch:%lf, pitch_conv:%lf\n", int_elevation, elevation_conv, int_pitch, pitch_conv);

	cs->int_travel +=  x.travel;
	cs->int_pitch +=  x.pitch;
	cs->int_elevation +=  x.elevation;

	double trav = 30.0 * (x.travel - sp.travel)/100.0;
	double d_trav = PERIOD*45*(x.d_travel )/10.0; 
	printf("controller d %lf\n",  trav) ; //30*(x.travel-spc.travel)/100.0 ) ; 	
	printf("controller   %lf\n",  d_trav) ; //2*(x.d_travel)/10.0 ) ; 

	//This one is working!
	//right voltage
//        U.u1 =    -6.5 * (x.elevation - sp.elevation)  - .701 * x.pitch + trav  - 45.7161 * PERIOD * x.d_elevation -3.051 * PERIOD * x.d_pitch +d_trav; //-0.0333*cs->int_elevation -0.001*cs->int_pitch;
        //left voltage
//	U.u2  =   -6.5 * (x.elevation - sp.elevation)  + .5701 * x.pitch -trav - 45.7529 * PERIOD * x.d_elevation +5.970 * PERIOD*  x.d_pitch -d_trav; //-0.03*cs->int_elevation +0.001*cs->int_pitch;

        U.u1 =    -6.5 * (x.elevation - sp.elevation)  - .9701 * x.pitch + trav  - 55.7161 * PERIOD * x.d_elevation -7.051 * PERIOD * x.d_pitch +d_trav; //-0.0333*cs->int_elevation -0.001*cs->int_pitch;
        //left voltage
	U.u2  =   -6.5 * (x.elevation - sp.elevation)  + .97701 * x.pitch -trav - 55.7529 * PERIOD * x.d_elevation +10.970 * PERIOD*  x.d_pitch -d_trav; //-0.03*cs->int_elevation +0.001*cs->int_pitch;

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

	matrix_mult(Hx, X, HX_SIZE, 1, 6);

	int all_small = 1;
	int k = 0;
	for (k = 0 ; k < HX_SIZE ; k++){
		if(C[k][0] > hx[k][0]){

			all_small = 0;
/*
			int rr = 0 ;
			printf("\n\n\n");
			printf("%d\n", k);
			for ( rr = 0 ; rr < HX_SIZE; rr++){
				printf ("%d \t  hx: %f rr: %lf\n", rr, hx[rr][0], C[rr][0]);
			}
*/

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
	printf("sim1 init: elevation: %lf, pitch: %lf, travel: %lf, d_elevation: %lf, d_pitch: %lf, d_travel: %lf\n", init_state.elevation, init_state.pitch, init_state.travel, init_state.d_elevation, init_state.d_pitch, init_state.d_travel);

	int steps = time/SIM_STEP;
	int k = 0;
	for (k = 0; k <steps; k++){
		state_x = eval_state(state_x, U);
		if(check_safety(state_x) == 0)
		{
			state_x.safe = 0;
			printf("sim1 nsafe: elevation: %lf, pitch: %lf, travel: %lf, d_elevation: %lf, d_pitch: %lf, d_travel: %lf, step: %d, tot_steps: %d\n", state_x.elevation, state_x.pitch, state_x.travel, state_x.d_elevation, state_x.d_pitch, state_x.d_travel, k , steps);


			return state_x;
		}
	}
	state_x.safe = 1;

	printf("sim1 safe: elevation: %lf, pitch: %lf, travel: %lf, d_elevation: %lf, d_pitch: %lf, d_travel: %lf\n", state_x.elevation, state_x.pitch, state_x.travel, state_x.d_elevation, state_x.d_pitch, state_x.d_travel);

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
		struct command U = controller_safety(sps, state_x, &cs);
		state_x = eval_state(state_x, U);
		if ( check_safety(state_x) == 0){

			state_x.safe = 0;
			printf("sim2 nsafe: elevation: %lf, pitch: %lf, travel: %lf, d_elevation: %lf, d_pitch: %lf, d_travel: %lf\n", state_x.elevation, state_x.pitch, state_x.travel, state_x.d_elevation, state_x.d_pitch, state_x.d_travel);


			return state_x;
		}
	}

	state_x.safe = 1;
	printf("sim2 safe: elevation: %lf, pitch: %lf, travel: %lf, d_elevation: %lf, d_pitch: %lf, d_travel: %lf\n", state_x.elevation, state_x.pitch, state_x.travel, state_x.d_elevation, state_x.d_pitch, state_x.d_travel);

	return state_x;
}




//The outcome determines weather the safety controller should be used or not
int decide(struct state current_state, struct command U, double time){
	/*
	we are checking if the restart happens, the system after being under the fixed control for restart time and then 
	being controlled the safety controller, can be brought back into the safe region.
	*/
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

	struct state spc; //set point for safety and complex controllers
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



		printf("\n step: %d, sp.travel = %lf, travel = %lf, pitch = %lf, elevation = %lf, left: %lf, right: %lf\n", step, spc.travel, cs.travel, cs.pitch , cs.elevation, vol_left, vol_right); 

		if (record == 1){
			fprintf(ofp, "%d\t%d\t%d\t%d\t%lf\t%lf\n", step, travel, pitch, elevation, vol_right, vol_left);
		}

		if (step < 30 ){
			vol_right = 0;
			vol_left = 0;
		}
		else 
		{
		/*
			The logic is that after every restart the safety controller is active for a set amount of time. After that if the
			complex controller's command is safe, it can be used again.
		*/

			if (step > 1800)
			{
				spc.travel = 3.1415/2;
			}
			else if( step > 1600){
				spc.travel = 3.1415/4;
			}
			else if (step > 1400){
				spc.travel = 0;
			}
			else if (step > 900){
				spc.travel = 3.14;
			}
			else if (step > 400){
				spc.elevation = 0.4;
				spc.travel = 3.14/2;
			}
			else if (step > 300){
				sps.elevation = 0.4;
			}
			else if (step > 200){
				sps.elevation = 0.3;
			}
			else if (step > 150){
				sps.elevation = 0.2;
			}
			else if (step > 100){
				sps.elevation = 0.1;
			}
			else if (step >50){
				sps.elevation = 0;
			}
		
			struct command U_safety = controller_safety(sps, cs, &storage_safety);
			struct command U_complex = controller_complex(spc, cs,  &storage_complex);		
			printf("remaining_cycle %d\n", remaining_safety_cycles);

			if (step < 400){
			        vol_right = U_safety.u1;
                                vol_left = U_safety.u2;
			}
			else if(decide(cs, U_complex, 0.2) == 1 && (remaining_safety_cycles <= 0 )  ) {
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


//			vol_right = 0;
//			vol_left = 0;
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



