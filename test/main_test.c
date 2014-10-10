#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <math.h>
#include "acceleration.h"


#define __NR_set_acceleration 378
#define __NR_accevt_create 379
#define __NR_accevt_wait 380
#define __NR_accevt_signal 381
#define __NR_accevt_destroy 382

#define TIME_BETWEEN_POLLS 10

#define NUM_PROCS 10
#define SET_FRQ 5

int set_acceleration(struct dev_acceleration  *acceleration)
{
        return syscall(__NR_set_acceleration, acceleration);
}

int accevt_create(struct acc_motion *acceleration)
{
        return syscall(__NR_accevt_create, acceleration);
}

int accevt_wait(int event_id)
{
        return syscall(__NR_accevt_wait, event_id);
}

int accevt_signal(struct dev_acceleration * acceleration)
{
        return syscall(__NR_accevt_signal, acceleration);
}

int accevt_destroy(int event_id)
{
        return syscall(__NR_accevt_destroy, event_id);
}


int main(int argc, char ** argv)
{
	
	int fork_ctr = 0;
	pid_t pid;
	int set_freq = SET_FRQ;
	struct acc_motion myAcceleration;
	int event_id = 0;
	for(fork_ctr = 0; fork_ctr < NUM_PROCS; fork_ctr++)
	{
		pid = fork();
		if (pid == 0)
			break;
	
	}
	
	if (pid == 0 ) {
	srand(getpid());	
		
	myAcceleration.dlt_x = rand()%100;
	myAcceleration.dlt_y = rand()%100;
	myAcceleration.dlt_z = rand()%100;
	myAcceleration.frq = set_freq;
	event_id = accevt_create(&myAcceleration);
	printf("%d %d %d %d %d %d\n", getpid(), myAcceleration.dlt_x, myAcceleration.dlt_y, myAcceleration.dlt_z, myAcceleration.frq, event_id);
	
	int wait_ret = 0;
	wait_ret = accevt_wait(event_id);


	/*Tested by sending the signals here using the signaler binary*/

	wait_ret = accevt_destroy(event_id);

	exit(0);

	}
	else {

	//	wait(&pid);
		exit(0);
	}



	return 0;			
}
