#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

MODULE_LICENSE("Dual BSD/GPL");

int Q8_Initialise (int BoardNum);
int Q8_DACConfigure(int BoardNum, unsigned char Chan, char Bipolar, int Range);
int Q8_EncInitialise(int BoardNum, unsigned char Chan, char BCD, unsigned char Mode, 
					 unsigned char Quadr, char IndexEn, char IndexPolarity);

static __init int hello_init(void)		
{
	printk("Q8 Test Module Loading:\n");
	Q8_Initialise(0);					// initialise Q8 Board 0
	Q8_DACConfigure(0, 4, 1, 10);		// configure the DAC: brd 0, channel 4, bipolar, 10V range
	Q8_EncInitialise(0,1,0,0,1,1,1);	// configure the encoder: brd 0, encoder 1, single quad, index & polarity set

	printk("Q8 Test Module Running:\n"); //printk(KERN_ALERT "Q8 Test Module Running:\n");
    return 0;
}

static __exit void hello_exit(void)
{
	printk(KERN_ALERT "Q8 Test Module removed.\n");
}

module_init(hello_init);
module_exit(hello_exit);

