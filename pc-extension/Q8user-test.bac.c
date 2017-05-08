//#include "DataTypes.h"
#include "q8user-betti.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define DRIVER_NAME "./test"
#define Q8_DAC_RESOLUTION  (12)
#define Q8_DAC_ZERO        (0x0800)
#define DEBUG 1

unsigned short int Q8_dacVTO( double nVoltage, int bBipolar, double nRange )
{
    if (bBipolar)
    {
        const double nOneLSB = 2 * nRange / (1 << Q8_DAC_RESOLUTION); // volts per LSB
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
        const double nOneLSB = nRange / (1 << Q8_DAC_RESOLUTION); // volts per LSB
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
	int tmp;

	/* .................Opening the device (Read/Write)................*/

	printf("\n\tOpening the device ..please wait...\n");
	fflush(stdout);

	File_Descriptor = open(DRIVER_NAME, O_RDWR);

	if( File_Descriptor < 0 ) /*Display a message if any error occures*/
	{
		perror("open");
		fprintf( stderr, "\n\tCould not open device file - Error : %i\n", File_Descriptor );

		return -1;
	}

	ioctl(File_Descriptor, Q8_IOCTL_EXAMPLE1);
	ioctl(File_Descriptor, Q8_IOCTL_EXAMPLE2, Buffer);
	for (i = 0; i < Q8_EXAMPLE2_MAXLEN; i ++)
		printf ("%c", Buffer[i]);
	printf ("\n");
	ioctl(File_Descriptor, Q8_IOCTL_EXAMPLE3, 5);
	//ioctl(File_Descriptor, Q8_RD_ENC1, &tmp);
	double printme;
	//printme = tmp * 0.0439;
	//printf ("Encoder Returned Value = %d\n", tmp);
	//printf ("Encoder Returned Value = %f degrees\n", printme);
	unsigned short int tmparray[4];
	for (i = 0; i < 4; i++)
	{
		tmparray[i] = Q8_dacVTO((1.0*i), 1, 10);
		printf("DA CH %f = %x\n", (1.0*i), tmparray[i]);
	}
	ioctl(File_Descriptor, Q8_WR_DAC, tmparray);
	
	return 0;
}

int get_enc (int channel)	{
	int tmp;
	fflush(stdout);

	File_Descriptor = open(DRIVER_NAME, O_RDWR);

	if( File_Descriptor < 0 ) /*Display a message if any error occures*/
	{
		perror("open");
		fprintf( stderr, "\n\tCould not open device file - Error : %i\n", File_Descriptor );

		return -1;
	}
	ioctl(File_Descriptor, Q8_RD_ENC1, &tmp);
	return tmp;
}
