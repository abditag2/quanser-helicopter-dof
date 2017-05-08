#include <rk/rk.h>
#include <time.h>
#include <stdio.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <math.h>

#define MSEC_TO_NSEC 1000000
#define USEC_TO_NSEC 1000

#define TASK1_PED 2

//unsigned char timevect[100*BUFLEN];
unsigned long int starttime;
//unsigned long int loopvect[100*BUFLEN];
//char array[ARRAY_SIZE];

unsigned int loop_cnt;
typedef unsigned char BUF;
BUF inbuf[ARRAY_SIZE];
//BUF outbuf[BUFLEN];

int child1()
{
   rk_resource_set_t rs;
	struct cpu_reserve_attr cpu_attr;
	FILE *f;
	unsigned long long  now;
   //unsigned long int curr_time, rel_time;
	long start_time, exec_time[LOOP_NUM];
	long avg_exec_time;
	double std_exec_time;
   register int i,j,k,l;
   register BUF *in, *out;
   
   in=inbuf;

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
	rs = rk_resource_set_create("RSETA0");
	rk_cpu_reserve_create(rs, &cpu_attr);


	for(loop_cnt=0;loop_cnt<LOOP_NUM;loop_cnt++){	
		rt_wait_for_next_period();
	  	rk_get_current_time(&now);
		start_time=now/USEC_TO_NSEC;
		/* And this is where we sample the encoders... */
		Q8_EncReadSingleLatch (0, 0);
		Q8_EncReadSingleLatch (0, 1);
		Q8_EncReadSingleLatch (0, 2);
		rk_get_current_time(&now);
		if(now/USEC_TO_NSEC<start_time) 
			printf("illegal\n");
	   exec_time[loop_cnt]=now/USEC_TO_NSEC - start_time;
	}
	i=rk_resource_set_destroy(rs);
	printf("child1 is done: %d \n",i);
	return 0;
}

int main()
{
	int pid1, res1;
	struct timespec now;
   register int x;
	//clock_gettime(CLOCK_REALTIME, &now);
	//starttime = now.tv_nsec / 1000000 + now.tv_sec * 1000; 

   //fill in the buffer
   /*for(x=0;x<BUFLEN;x++){
      inbuf[x]=(BUF)(x & 0xff);
   }*/
	printk("Start...\n");
	if((pid1=fork())==0)
	{
		child1();
	}
	wait(&res1);	
	return 0;
}
