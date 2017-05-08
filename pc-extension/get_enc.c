#include "/usr/src/Q8Package-betti/q8user-betti.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define Q8_DAC_RESOLUTION  (12)
#define Q8_DAC_ZERO        (0x0800)
#define DEBUG 0
int File_Descriptor = 0;
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
/*
int get_enc (int channel)	{
	int tmp;
	if (File_Descriptor == 0)
	{
		fflush(stdout);
		File_Descriptor = open(DRIVER_NAME, O_RDWR);

		if( File_Descriptor < 0 ) /*Display a message if any error occures*/ /*
		{
			perror("open");
			fprintf( stderr, "\n\tCould not open device file - Error : %i\n", File_Descriptor );
			return -1;
		}
	}
	ioctl(File_Descriptor, Q8_RD_ENC1, &tmp);
	return tmp;
}
*/
int get_enc (int channel)	{
	int tmp;
	int err;
	if (File_Descriptor == 0)
	{
		fflush(stdout);
		File_Descriptor = open(DRIVER_NAME, O_RDWR);

		if( File_Descriptor < 0 ) /*Display a message if any error occures*/
		{
			perror("open");
			fprintf( stderr, "\n\tCould not open device file - Error : %i\n", File_Descriptor );
			return -1;
		}
	}
	switch (channel)
	{
		case 0:
		{
			err = ioctl(File_Descriptor, Q8_ENC_0, &tmp);
			if (err != 0)
			{
				fprintf(stderr, "Epic Fail channel 0");
				return -1;
			}

		}
		break;
		case 1:
		{
			err = ioctl(File_Descriptor, Q8_ENC_1, &tmp);
			if (err != 0)
			{
				fprintf(stderr, "Epic Fail channel 1");
				return -1;
			}
		}
		break;
		case 2:
		{
			err = ioctl(File_Descriptor, Q8_ENC_2, &tmp);
			if (err != 0)
			{
				fprintf(stderr, "Epic Fail channel 2");
				return -1;
			}
		}
		break;
		default:
			return -1; /* cmd not valid */
	}
	return tmp;
}
int Q4_Enc(int* array)
{
	int err;	
	if (File_Descriptor == 0)
	{
		fflush(stdout);
		File_Descriptor = open(DRIVER_NAME, O_RDWR);

		if( File_Descriptor < 0 ) /*Display a message if any error occures*/
		{
			perror("open");
			fprintf( stderr, "\n\tCould not open device file - Error : %i\n", File_Descriptor );
			return -1;
		}
	}
	err = ioctl(File_Descriptor, Q8_ENC, array);
	if (err != 0)
	{
		fprintf(stderr, "Epic Fail on Q4_Enc");
	}
	return err;
}

int wrt_dacs (int channel, int val)
{
	int tmp;
	if (File_Descriptor == 0)
	{
		fflush(stdout);
		File_Descriptor = open(DRIVER_NAME, O_RDWR);

		if( File_Descriptor < 0 ) /*Display a message if any error occures*/
		{
			perror("open");
			fprintf( stderr, "\n\tCould not open device file - Error : %i\n", File_Descriptor );
			return -1;
		}
	}
	switch (channel)
	{
		case 0:
		{
			tmp = ioctl(File_Descriptor, Q8_DAC_0, val);
		}
		break;
		case 1:
		{
			tmp = ioctl(File_Descriptor, Q8_DAC_1, val);
		}
		break;
		default:
			return -1; /* cmd not valid */
	}
	if (tmp != 0)
	{
		return -1;
	}
	else
	{
		return 0;
	}	
}

int Q4_Dac (double* array)
{
	short int tmp[2];
	int err;
	double val;
	if (File_Descriptor == 0)
	{
		fflush(stdout);
		File_Descriptor = open(DRIVER_NAME, O_RDWR);

		if( File_Descriptor < 0 ) /*Display a message if any error occures*/
		{
			perror("open");
			fprintf( stderr, "\n\tCould not open device file - Error : %i\n", File_Descriptor );
			return -1;
		}
	}
	val = array[0];
	tmp[0] =  Q8_dacVTO(val, 1, 10 );
	val = array[1];
	tmp[1] =  Q8_dacVTO( val, 1, 10 );
	err = ioctl(File_Descriptor, Q8_WR_DACS, tmp);
	if (err != 0)
	{
		fprintf(stderr, "Epic Fail Q4_Dac");
	}
	return err;
}

