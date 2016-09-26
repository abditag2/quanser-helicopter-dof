#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <rk/rk.h>
#include <linux/time.h>
//#include </include/linux/stdio.h>
#include "/usr/src/linux-2.6.18/include/asm-i386/resource.h"
#include "/usr/src/linux-2.6.18/include/rk/rk.h"
//#include <linux/stdlib.h>
//#include <linux/math.h>

#define MSEC_TO_NSEC 1000000
#define USEC_TO_NSEC 1000

#define TASK1_PED 2

//unsigned char timevect[100*BUFLEN];
unsigned long int starttime;
//unsigned long int loopvect[100*BUFLEN];
//char array[ARRAY_SIZE];

unsigned int loop_cnt;
typedef unsigned char BUF;
MODULE_LICENSE("Dual BSD/GPL");
int rt_wait_for_next_period(void);
void rk_get_current_time(unsigned long long *);
int Q8_EncReadSingleLatch(int BoardNum, int nChannel);
int Q8_Initialise (int BoardNum);
int Q8_DACConfigure(int BoardNum, unsigned char Chan, char Bipolar, int Range);
int Q8_EncInitialise(int BoardNum, unsigned char Chan, char BCD, unsigned char Mode, 
					 unsigned char Quadr, char IndexEn, char IndexPolarity);
int child1(void);

int child1()
{
   rk_resource_set_t rs;
	struct cpu_reserve_attr cpu_attr;
	unsigned long long  now;
   register int i,j,k,l;

   char flag=0;
	/*loop_count counts the number of loops having run in each period of reserved CPU resource
   *run_count counts the number of periods of reserved CPU resource have been used by the task*/
   unsigned int loop_cnt=0, run_count=0;
   float avg_loop_count=0;

   cpu_attr.compute_time.tv_sec=1;
	cpu_attr.period.tv_sec=TASK1_PED;
	cpu_attr.deadline.tv_sec=TASK1_PED;
	cpu_attr.blocking_time.tv_sec=0;
	cpu_attr.start_time.tv_sec=0;

	cpu_attr.compute_time.tv_nsec=0;//10000000;
	cpu_attr.period.tv_nsec=0;//40000000;
	cpu_attr.deadline.tv_nsec=0;//40000000;
	cpu_attr.blocking_time.tv_nsec=0;
	cpu_attr.start_time.tv_nsec=0;
	
	
	cpu_attr.reserve_type.sch_mode = RSV_HARD;
	cpu_attr.reserve_type.enf_mode = RSV_HARD;
	cpu_attr.reserve_type.rep_mode = RSV_HARD;
	
#ifdef HARDWARE_SERVER
	cpu_attr.hw_mask = 3;
	cpu_attr.task_id = 0;
#endif	

	//printf("%d\n", getpid());
	//rs = rk_resource_set_create("RSETA0");
	//rk_cpu_reserve_create(rs, &cpu_attr);


	//for(loop_cnt=0;loop_cnt<LOOP_NUM;loop_cnt++){	
	while (1)	{
		rt_wait_for_next_period();
	  	rk_get_current_time(&now);
		/* And this is where we sample the encoders... */
		Q8_EncReadSingleLatch (0, 0);
		Q8_EncReadSingleLatch (0, 1);
		Q8_EncReadSingleLatch (0, 2);
		rk_get_current_time(&now);
	}
	i=rk_resource_set_destroy(rs);
	printk("child1 is done: %d \n",i);
	return 0;
}
static __init int hello_init(void)		
{
	int pid1, res1;
	struct timespec now;
   	register int x;

	printk("Q8 Test Module Loading:\n");
	Q8_Initialise(0);					// initialise Q8 Board 0
	Q8_DACConfigure(0, 4, 1, 10);		// configure the DAC: brd 0, channel 4, bipolar, 10V range
	Q8_EncInitialise(0,0,0,0,1,1,1);	// configure the encoder: brd 0, encoder 1, single quad, index & polarity set
	if((pid1=fork())==0)
	{
		child1();
	}
	printk("Q8 Test Module Running:\n"); //printk(KERN_ALERT "Q8 Test Module Running:\n");
	wait(&res1);	
    return 0;
}

static __exit void hello_exit(void)
{
	printk(KERN_ALERT "Q8 Test Module removed.\n");
}

module_init(hello_init);
module_exit(hello_exit);

