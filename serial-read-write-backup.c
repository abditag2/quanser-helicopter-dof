#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>

#define SERIAL_DATA_SIZE 8

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

void set_blocking (int fd, int should_block)
{
	struct termios tty;
	memset (&tty, 0, sizeof tty);
	if (tcgetattr (fd, &tty) != 0)
	{
		//printf("error %d from tggetattr", errno);
		return;
	}

	tty.c_cc[VMIN]  = should_block ? 8 : 0;
	tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

	if (tcsetattr (fd, TCSANOW, &tty) != 0){
		//printf("error %d setting term attributes", errno);
	}
}


void read_8_bytes(int fd, int* commands){
	char buf8[SERIAL_DATA_SIZE];
	char buf;

	while(1){
		int n = read (fd, &buf, 1);

		if (n != 1) {
			printf("n is not 1, n is: %d\n", n);
		}
		else{
			if (buf == -86){
				int i;
				for (i = 0 ; i < SERIAL_DATA_SIZE ; i++)
				{
					n = read (fd, &buf, 1);	
					buf8[i] = buf;
				}	

				break;

				//				for (i = 0 ; i < 9 ; i ++)
				//				{
				//					printf("%x\n", buf8[i]);
				//				}

			}

		}

	}

	int vol_left = *(int*)buf8;
	int vol_right = *(int*) &buf8[4];

	commands[0] = vol_left;
	commands[1] = vol_right;

}


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
		Txbuffer[4*i+1]            = (char) (sensor_readings[i] & 0x000000ff); /* first byte */
		Txbuffer[4*i + 2]          = (char) (sensor_readings[i] & 0x0000ff00) >> 8; /* second byte */
		Txbuffer[4*i + 3]          = (char) (sensor_readings[i] & 0x00ff0000) >> 16; /* third byte */
		Txbuffer[4*i + 4]          = (char) (sensor_readings[i] & 0xff000000) >> 24; /* fourth byte */
	}
	Txbuffer[13] = 0xFF;

	for ( i = 0 ; i < 14 ; i++){
		write(fd, Txbuffer[i], 1);
	}

	usleep (2000);             // sleep enough to transmit the 7 plus

	// receive 25:  approx 100 uS per char transmit


	char buf [1];
	tcflush(fd,TCIOFLUSH);
	int commands[2];
	read_8_bytes(fd, commands);
	printf("commands[0]: %x\n", commands[0]);
	/*
	   int n = read (fd, buf, 1);  // read up to 100 characters if ready to read

	   if (n != 1) {
	   printf("n is not 1, n is: %d\n", n);
	   }
	   else{
	   char a[4];
	   char b[4];

	   a[3] = buf[0];
	   a[2] = buf[1];
	   a[1] = buf[2];
	   a[0] = buf[3];

	   int vol_left = (int) buf[0];

	   b[3] = buf[4];
	   b[2] = buf[5];
	   b[1] = buf[6];
	   b[0] = buf[7];

	   int vol_right = (int) buf[4];
	   int i = 0;
	   for (i = 0 ; i < 8 ; i ++)
	   {
	   printf("%x\n", buf[i]);
	   }
	//usleep(1000);

	}
	 */
}	




