#include "q8user-betti.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


/*  #define DRIVER_NAME "./test"*/
#define Q8_DAC_RESOLUTION  (12)
#define Q8_DAC_ZERO        (0x0800)
#define DEBUG 1

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
int main()
{
	int File_Descriptor, i, j; /**/
	int Return_Value ; /*To catch the return value .......*/
	char Buffer[Q8_EXAMPLE2_MAXLEN] ;
	int tmp = 0;
	int travel = -1;
	int pitch = -1;
	int elevation = -1;
	int err;

	/* .................Opening the device (Read/Write)................*/

	printf("\n\tOpening the device ..please wait...\n");
	fflush(stdout);

	File_Descriptor = open(DEVICE_FILE_NAME, O_RDWR); 

	if( File_Descriptor < 0 ) /*Display a message if any error occures*/ 
	{
		perror("open");
		fprintf( stderr, "\n\tCould not open device file - Error : %i\n", File_Descriptor );

		return -1;
	}

	int johny[3];
	/*travel = get_enc(0);
	sleep(1);
	pitch = get_enc(1);
	sleep(1);
	elevation = get_enc(2);*/
	err = ioctl(File_Descriptor, Q8_ENC, johny);
	if(err != 0)
	{
		perror("Epic Fail first enc read\n");
		return -1;
	}

	printf("\n travel = %x, pitch = %x, elevation = %x\n", johny[0], johny[1], johny[2]); 
	//printf(" value = %x\n", elevation);
	close(DEVICE_FILE_NAME);
	return 0;
}
