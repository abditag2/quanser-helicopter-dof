#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

#define DAC_CH_MASK 0x03
#define ENC_CH_MASK 0x07
#define DAC_CH_SIZE 2
#define ENC_CH_SIZE 3
#define ENC_FLGS 0x03
#define DAC_RNG 10
#define BIPOLAR 0
int Q8_Initialise (int BoardNum);
int Q8_EncInitialise(int BoardNum, unsigned char Chan, char BCD, unsigned char Mode, 
					 unsigned char Quadr, char IndexEn, char IndexPolarity);
void Q8_EncSetFilterPrescaler (int BoardNum, int nChannel, int nValue);
void Q8_EncConfigureIO (int BoardNum, int nChannel, int nFlags, int bLoadCounterOnIndex);
int Q8_DACPreConfigure (int BoardNum, unsigned char nChannel, char bBipolar, int nRange);
unsigned short int Q8_DACVoltageToOutput(double nVoltage, unsigned char bBipolar, double nRange );
int Q8_SetDAC(int BoardNum, char ChanMask, short int *v);


int Q4Init()		
{
	int err;
	int i;
	short int val[DAC_CH_SIZE];
	err = Q8_Initialise(0);
	if (err != 0)
	{
		printk(KERN_ALERT "Q4 could not be Initialized, board not present");
		return -1;
	}
	for (i = 0; i < ENC_CH_SIZE; i++)
	{
		err = Q8_EncInitialise(0, i, 0, 0, 3, 1, 1);
		if (err != 0)
		{
			printk(KERN_ALERT "Q4 ENC %d could not be Initialized", i);
			return -1;
		}
	}
	for (i = 0; i < ENC_CH_SIZE; i++)
	{
		Q8_EncSetFilterPrescaler(0, i, 0);
	}
	for (i = 0; i < ENC_CH_SIZE; i++)
	{
		Q8_EncConfigureIO(0, i, ENC_FLGS, 0);
	}
	for (i = 0; i < ENC_CH_SIZE; i++)
	{
		err = Q8_DACPreConfigure(0, i, BIPOLAR, DAC_RNG);
		if (err != 0)
		{
			printk(KERN_ALERT "Q4 DAC %d could not be Initialized", i);
			return -1;
		}
	}
	for (i = 0; i < DAC_CH_SIZE; i++)
	{
		val[i] = Q8_DACVoltageToOutput(0, BIPOLAR, DAC_RNG);
	}
	err = Q8_SetDAC(0, DAC_CH_MASK, val);
	if (err != 0)
	{
		printk(KERN_ALERT "Q4 DAC's could not be set to Zero for open\n"); 	
		return -1;
	}
	printk(KERN_ALERT "Q4 Has been Initialized\n");
	return 0;
}

int Q4Exit()
{
	int err;
	short int val[DAC_CH_SIZE];
	int i;
	for (i = 0; i < DAC_CH_SIZE; i++)
	{
		val[i] = Q8_DACVoltageToOutput(0, BIPOLAR, DAC_RNG);
	}
	err = Q8_SetDAC(0, DAC_CH_MASK, val);
	if (err != 0)
	printk(KERN_ALERT "Q4 DAC's could not be set to Zero for close\n"); 
	printk(KERN_ALERT "Q4 has been Uninitialized.\n");
}

module_init(hello_init);
module_exit(hello_exit);

