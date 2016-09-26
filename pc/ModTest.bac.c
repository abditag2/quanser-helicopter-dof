#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

#include <asm/rtai.h>
#include <rtai_sched.h>

MODULE_LICENSE("Dual BSD/GPL");

int Q8_Initialise (int BoardNum);
int Q8_SetDAC(int BoardNum, char ChanMask, short int *v);
int Q8_DACConfigure(int BoardNum, unsigned char Chan, char Bipolar, int Range);
int Q8_DACPreConfigure (int BoardNum, unsigned char nChannel, char bBipolar, int nRange);
int Q8_EncInitialise(int BoardNum, unsigned char Chan, char BCD, unsigned char Mode, 
					 unsigned char Quadr, char IndexEn, char IndexPolarity);
long Q8_EncInputSingle(int BoardNum, unsigned char Chan);
unsigned char  Q8_ADCInput(int BoardNum, unsigned char nChan, short int *nValues, unsigned char bSimultaneous);
unsigned short int Q8_DACVoltageToOutput(double nVoltage, unsigned char bBipolar, double nRange );
double Q8_ADCInputToVoltage(short int value);

#define STACK_SIZE    4000
#define TICK_PERIOD   15000
#define PERIOD_COUNT  1
#define PRIORITY      0
#define USE_FPU       0

RT_TASK thread;

static void fun(long cookie)// sample function for Q8 demonstration
{
	int i;
	short int V[8];			// array to store DAC values prior to writing them to the DAC
	short int AD[8];		// stored ADC readings
	double fAD[8];			// same as above but for real values
	long EV;				// an encoder reading

    while (1)
    {
		Q8_ADCInput(0,3,AD,1);									// get the ADC values from channels 0,1 (00000011 = 3)
		for(i = 0; i < 8; i++)									// for each ADC channel
			fAD[i] = Q8_ADCInputToVoltage(AD[0]);				// convert them to reals

		EV = Q8_EncInputSingle(0,1);							// read Encoder 1
		for(i = 0; i < 8; i++)									// for each DAC channel
			V[i] = Q8_DACVoltageToOutput(fAD[i], 1, 10) + EV;	// set the appropriate value

		Q8_SetDAC(0,0x1F,V);									// write to DAC chans 0 - 5 (00011111 = 0x1F);

        rt_task_wait_period();									// rt wait
    }
}

static __init int hello_init(void)		
{
    RTIME tick_period;
    RTIME now; 

	Q8_Initialise(0);					// initialise Q8 Board 0
	Q8_DACConfigure(0, 4, 1, 10);		// configure the DAC: brd 0, channel 4, bipolar, 10V range
	Q8_EncInitialise(0,1,0,0,1,1,1);	// configure the encoder: brd 0, encoder 1, single quad, index & polarity set

    rt_task_init(&thread, fun, 0, STACK_SIZE, PRIORITY, USE_FPU, NULL);	// init the rt thread

    tick_period = start_rt_timer(nano2count(TICK_PERIOD));	// start the timer
    now = rt_get_time();
    rt_task_make_periodic(&thread, now + tick_period, tick_period*PERIOD_COUNT);

	printk(KERN_ALERT "Q8 Test Module Running:\n");
	printk(KERN_ALERT "DAC4 mirroring voltage on ADC0 + encoder value.\n");
    return 0;
}

static __exit void hello_exit(void)
{
    stop_rt_timer();
    rt_busy_sleep(10000000);
    rt_task_delete(&thread);
	printk(KERN_ALERT "Q8 Test Module removed.\n");
}

module_init(hello_init);
module_exit(hello_exit);

