	#include "q8user-betti.h"
	#include <stdio.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <fcntl.h>
	#include <time.h>
	#include <stdint.h>
	#include <inttypes.h>


	/*  #define DRIVER_NAME "./test"*/
	#define Q8_DAC_RESOLUTION  (12)
	#define Q8_DAC_ZERO        (0x0800)
	#define DEBUG 0

	#include <errno.h>
	#include <fcntl.h> 
	#include <string.h>
	#include <termios.h>
	#include <unistd.h>
	#include <stdio.h>

	#define SERIAL_DATA_SIZE 9
	#define MAX_VOLTAGE 3.0

	int set_interface_attribs (int fd, int speed, int parity)
	{

		struct termios tty;
		memset (&tty, 0, sizeof tty);
		if (tcgetattr (fd, &tty) != 0)
		{
			//                printf("error %d from tcgetattr", errno);
			return -1;
		}

		cfsetospeed (&tty, speed);
		cfsetispeed (&tty, speed);

		tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
		// disable IGNBRK for mismatched speed tests; otherwise receive break
		// as \000 chars
		tty.c_iflag &= ~IGNBRK;         // disable break processing
		tty.c_lflag = 0;                // no signaling chars, no echo,
		// no canonical processing
		tty.c_oflag = 0;                // no remapping, no delays
		tty.c_cc[VMIN]  = 0;            // read doesn't block
		tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

		tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

		tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
		// enable reading
		tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
		tty.c_cflag |= parity;
		tty.c_cflag &= ~CSTOPB;
		tty.c_cflag &= ~CRTSCTS;

		if (tcsetattr (fd, TCSANOW, &tty) != 0)
		{
			//printf("error %d from tcsetattr", errno);
			return -1;
		}
		return 0;
	}

	void set_blocking (int fd, int should_block, int number_of_char)
	{
		struct termios tty;
		memset (&tty, 0, sizeof tty);
		if (tcgetattr (fd, &tty) != 0)
		{
			//printf("error %d from tggetattr", errno);
			return;
		}

		tty.c_cc[VMIN]  = should_block ? number_of_char : 0;
		tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

		if (tcsetattr (fd, TCSANOW, &tty) != 0){
			//printf("error %d setting term attributes", errno);
		}
	}


	void read_8_bytes(int fd, double* commands){
		// printf("READ\n");
		unsigned char buf8[SERIAL_DATA_SIZE];
		unsigned char buf;

		while(1){
			set_blocking (fd, 1, 1); 
			int n = read (fd, &buf, 1);
			
			if (n != 1) {
			//	printf("n is not 1, n is: %d\n", n);
			}
			else{
	//			printf("READ: char read: %x\n", buf); 
				if (buf == 0xCC){
					int i;
	//				for (i = 0 ; i < SERIAL_DATA_SIZE ; i++)
	//				{
						set_blocking (fd, 1, SERIAL_DATA_SIZE); 
						n = read (fd, &buf8, SERIAL_DATA_SIZE);
	//					buf8[i] = buf;
	//				}
					printf("READ: recieved bytes:\n");

	//				for ( i = 0; i < SERIAL_DATA_SIZE; i++)
	//				{
	//					printf("READ: %x\n",buf8[i]);
	//				}	

					break;

				}

			}

		}

		int vol_left = *(int*)buf8;
		int vol_right = *(int*) &buf8[4];
		
		printf("vol left : %d \n", vol_left);	

		commands[0] = ((double) vol_left)/10000.0;
		commands[1] = ((double) vol_right)/10000.0;

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

	double vol_right = 0;
	double vol_left = 0;

	void send_serial(int fd, int sensor_readings[]){
		
		

		unsigned char Txbuffer[13];
		int i;
		unsigned char buf;

		for (i = 0; i < 3; i ++ ) {
			Txbuffer[4*i]            = (unsigned char) (sensor_readings[i] & 0xff); /* first byte */
			Txbuffer[4*i + 1]          = (unsigned char) (sensor_readings[i] >> 8  & 0xff); /* second byte */
			Txbuffer[4*i + 2]          = (unsigned char) (sensor_readings[i] >> 16 & 0xff); /* third byte */
			Txbuffer[4*i + 3]          = (unsigned char) (sensor_readings[i] >> 24 & 0xff); /* fourth byte */
		}
		
//		unsigned char start = 0xAA;
		Txbuffer[12] = 0xFF;

		
		while (1){
	//		printf("SEND: in the while loop.\n");
	//		write(fd, &start, 1);

			set_blocking (fd, 1, 1); 			
			int n = read (fd, &buf, 1);
			if (n == 1) {
	//			printf("SEND: char is: %x\n", buf);
				if (buf == 0xAA){
	//				printf("SEND: I recieved BB\n");
					write(fd, Txbuffer, 13);
					usleep (1000);             // sleep enough to transmit the 7 plus
	//				printf("SEND: data is written.\n");
					break;
				}
			}
		}
		

	}

	/*
	void main(){

		char *portname = "/dev/ttyUSB1";
		int fd = open (portname, O_RDWR | O_NOCTTY | O_SYNC);
		if (fd < 0)
		{
			printf("error %d opening %s: %s\n", errno, portname, strerror (errno));
			return;
		}

		set_interface_attribs (fd, B115200, 0);  // set speed to 115,200 bps, 8n1 (no parity)
		set_blocking (fd, 1);                // set no blocking

		int sensor_readings[3];
		sensor_readings[0] = 1;
		sensor_readings[1] = 2;
		sensor_readings[2] = 3;

		char Txbuffer[14];
		int i;

		Txbuffer[0] = 0xAA;
		for (i = 0; i < 3; i ++ ) {
			Txbuffer[4*i+1]            = (char) (sensor_readings[i] & 0x000000ff); 
			Txbuffer[4*i + 2]          = (char) (sensor_readings[i] & 0x0000ff00) >> 8; 
			Txbuffer[4*i + 3]          = (char) (sensor_readings[i] & 0x00ff0000) >> 16;
			Txbuffer[4*i + 4]          = (char) (sensor_readings[i] & 0xff000000) >> 24;
		}
		Txbuffer[13] = 0xFF;

		for ( i = 0 ; i < 14 ; i++){
			write(fd, Txbuffer[i], 1);
		}

		usleep (2000);             // sleep enough to transmit the 7 plus

		char buf [1];
		tcflush(fd,TCIOFLUSH);
		int commands[2];
		read_8_bytes(fd, commands);
		printf("commands[0]: %x\n", commands[0]);
	}	
	*/


	int main()
	{
		//serial port init

		char *portname = "/dev/ttyUSB1";
		int fd = open (portname, O_RDWR | O_NOCTTY | O_SYNC);
		if (fd < 0)
		{
			printf("error %d opening %s: %s\n", errno, portname, strerror (errno));
			return;
		}

		set_interface_attribs (fd, B115200, 0);  // set speed to 115,200 bps, 8n1 (no parity)
		//set_blocking (fd, 1);                // set no blocking

	// quanser init

		int File_Descriptor; 
		int Return_Value ; /*To catch the return value .......*/
		char Buffer[Q8_EXAMPLE2_MAXLEN] ;
		int tmp;

		/* .................Opening the device (Read/Write)................*/

		printf("\n\tOpening the device ..please wait...\n");
		fflush(stdout);

		File_Descriptor = open(DEVICE_FILE_NAME, O_RDWR);
		if( File_Descriptor < 0 ) 
		{
			perror("open");
			fprintf( stderr, "\n\tCould not open device file - Error : %i\n", File_Descriptor );
			return -1;
		}

		/* .. Writing the control values to the left and write motor......*/

		int step = 0;
		double vol_step = 0.1; 	

		int sensor_reading[3];

		/* .. Reading the Encoder values from the helicopter......*/
		int err = ioctl(File_Descriptor, Q8_ENC, sensor_reading);
		if(err != 0)
		{
			perror("Epic Fail first enc read\n");
			return -1;
		}
	
		int base_travel = sensor_reading[0];
		int base_pitch = sensor_reading[1];
		int base_elevation = sensor_reading[2];

		
		// write
	/*	unsigned short int tmparray[4];
		tmparray[0] = Q8_dacVTO((vol_right), 1, 10);
		tmparray[1] = Q8_dacVTO((vol_left), 1, 10);
		ioctl(File_Descriptor, Q8_WR_DAC, tmparray);
	*/

		struct timeval current_time, last;
		

		while(1) {
		/* .. Reading the Encoder values from the helicopter......*/
		
			int err = ioctl(File_Descriptor, Q8_ENC, sensor_reading);
			if(err != 0)
			{
				perror("Epic Fail first enc read\n");
				return -1;
			}
			//close(File_Descriptor);

		//	sensor_reading[0] = 1000;
		//	sensor_reading[1] = 24000;

			//last = current_time;

			
			gettimeofday(&last, NULL);			

			int sensor_readings_calibrated[3];
			
			sensor_readings_calibrated[0] = sensor_reading[0] - base_travel;
			sensor_readings_calibrated[1] = sensor_reading[1] - base_pitch;
			sensor_readings_calibrated[2] = sensor_reading[2] - base_elevation;
			
			send_serial(fd, sensor_readings_calibrated);

			usleep(10000);
			//usleep(50000);
		// write
			double commands[2];
			read_8_bytes(fd, commands);

			
			gettimeofday(&current_time, NULL);
						
			uint64_t delta_us = (current_time.tv_sec - last.tv_sec) * 1000000 + (current_time.tv_usec - last.tv_usec);
			
			printf("\n \t\t\t%d, %d, %d\n", sensor_readings_calibrated[0], sensor_readings_calibrated[1], sensor_readings_calibrated[2]);	
			printf("%" PRId64 " \t val1: %lf, val2: %lf\n", delta_us, commands[0], commands[1] ) ;



			vol_left = commands[0];
			vol_right = commands[1];


				if (vol_right > MAX_VOLTAGE)
					vol_right = MAX_VOLTAGE;
				else if (vol_right < -MAX_VOLTAGE)
					vol_right = -MAX_VOLTAGE;

				if (vol_left > MAX_VOLTAGE)
					vol_left = MAX_VOLTAGE;
				else if (vol_left < -MAX_VOLTAGE)
					vol_left = -MAX_VOLTAGE;

			unsigned short int tmparray[4];
			
			
			//vol_right = 0;
			//vol_left = 0;
			
			tmparray[0] = Q8_dacVTO((vol_right), 1, 10);
			tmparray[1] = Q8_dacVTO((vol_left), 1, 10);
			ioctl(File_Descriptor, Q8_WR_DAC, tmparray);

	//		printf("\n step: %d, travel = %d, pitch = %d, elevation = %d, left: %lf, right: %lf\n", step, sensor_reading[0]-base_travel, sensor_reading[1] -base_pitch , -(sensor_reading[2]-base_elevation)- 350, vol_left, vol_right); 

		}
		
		return 0;
	}



