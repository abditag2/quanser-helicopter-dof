#include "q8user-betti.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>


/*  #define DRIVER_NAME "./test"*/
#define Q8_DAC_RESOLUTION  (12)
#define Q8_DAC_ZERO        (0x0800)
#define DEBUG 0

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

void* get_keyboard(void *arg){
	
	while(1){
		char in;
		scanf("%c", &in);
		if (in == '1'){
			printf("recording started\n");
			record = 1;
		}
		else if (in == '0'){
			record = 0;
			printf("recording ended\n");
		}
		sleep(1);
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
	double vol_right = 0;
	double vol_left = 0;

	double vol_step = 0.1; 	

	FILE *ofp;
	ofp = fopen("recorded_data.txt", "w");
	
	for (step = 0 ; step < 5000 ; step++){
		if (step < 400) {
			vol_right = -1.1;
			vol_left = 1.135;			
		}
		else if (step >= 400 && step < 1000) {
			vol_right = -1.2;
			vol_left = 1.245;
		}
		else if (step >=1000 && step <1050){
			vol_right = 1.3;
			vol_left = 1.34;
		}
		else if (step >= 1050 && step < 4000){
			vol_right = 1.3;
			vol_left = 1.33;
		}
		else if (step >= 4000){
			vol_right = 0;
			vol_left = 0;
		}

//		vol_right = 0 ;
//		vol_left = 0 ;

		unsigned short int tmparray[4];
		tmparray[0] = Q8_dacVTO((vol_right), 1, 10);
		tmparray[1] = Q8_dacVTO((vol_left), 1, 10);
		ioctl(File_Descriptor, Q8_WR_DAC, tmparray);

		int johny[3];
	
		/* .. Reading the Encoder values from the helicopter......*/
	
		int err = ioctl(File_Descriptor, Q8_ENC, johny);
		if(err != 0)
		{
			perror("Epic Fail first enc read\n");
			return -1;
		}
		printf("\n step: %d, travel = %d, pitch = %d, elevation = %d, right: %lf, left: %lf\n", step, johny[0], johny[1]-5034, johny[2], vol_right, vol_left); 
		if (record == 1){
			fprintf(ofp, "%d\t%d\t%d\t%d\t%lf\t%lf\n", step, johny[0], johny[1], johny[2], vol_right, vol_left);
		}
		usleep (10000);
	}
	fclose(ofp);	
	return 0;
}



