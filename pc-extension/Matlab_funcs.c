#include "q8user-betti.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int File_Descriptor = 0;

int get_enc (int channel)	{
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
			ioctl(File_Descriptor, Q8_ENC_0, &tmp);
		}
		break;
		case 1:
		{
			ioctl(File_Descriptor, Q8_ENC_1, &tmp);
		}
		break;
		case 2:
		{
			ioctl(File_Descriptor, Q8_ENC_2, &tmp);
		}
		break;
		default:
			return -1; /* cmd not valid */
	}
	return tmp;
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
