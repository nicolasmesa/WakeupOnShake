#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <math.h>
#include <time.h>
#include "acceleration.h"


#define __NR_set_acceleration 378
#define __NR_accevt_create 379
#define __NR_accevt_wait 380
#define __NR_accevt_signal 381
#define __NR_accevt_destroy 382

#define TIME_BETWEEN_POLLS 10

#define NUM_PROCS 3
#define SET_FRQ 5
#define NUM_EVENTS 3
#define MAX_DELTA 300
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

int accevt_signal(struct dev_acceleration *acceleration)
{
	return syscall(__NR_accevt_signal, acceleration);
}

int accevt_destroy(int event_id)
{
	return syscall(__NR_accevt_destroy, event_id);
}

int main(int argc, char **argv)
{
	int fork_ctr = 0;
	pid_t pid;
	static pid_t procid_arr[NUM_PROCS];
	static int eventid_arr[NUM_EVENTS];
	int set_freq = SET_FRQ;
	int event_id = 0;
	/*Creating the three classes of events*/
	struct acc_motion myAcceleration[3];

	srand(time(NULL));
	myAcceleration[0].dlt_x = rand() % MAX_DELTA;
	myAcceleration[0].dlt_y = 0;
	myAcceleration[0].dlt_z = 0;
	myAcceleration[0].frq = set_freq;
	srand(time(NULL));
	myAcceleration[1].dlt_x = 0;
	myAcceleration[1].dlt_y = rand() % MAX_DELTA;
	myAcceleration[1].dlt_z = 0;
	myAcceleration[1].frq = set_freq;

	srand(time(NULL));
	myAcceleration[2].dlt_x = rand() % MAX_DELTA;
	myAcceleration[2].dlt_y = rand() % MAX_DELTA;
	myAcceleration[2].dlt_z = 0;
	myAcceleration[2].frq = set_freq;
	int event_create_ctr = 0;

	for (event_create_ctr = 0; event_create_ctr < NUM_EVENTS;
				event_create_ctr++) {
		if (&myAcceleration[event_create_ctr])
			event_id = accevt_create(
				&myAcceleration[event_create_ctr]);
		if (event_id < 1) {
			printf("Unable to create event\n");
			exit(1);
		}
		printf("Created event: %d\n", event_id);
		eventid_arr[event_create_ctr] = event_id;
	}

	for (fork_ctr = 0; fork_ctr < NUM_PROCS; fork_ctr++) {
		pid = fork();
		if (pid < 0) {
			printf("Fork failed.\n");
			exit(1);
		} else if (pid == 0)
			break;
		procid_arr[fork_ctr] = pid;
			printf("Process created: %d %d\n", pid,
				procid_arr[fork_ctr]);
	}
	if (pid == 0) {
		int wait_ret = 0;

		printf("%d waiting on %d\n", getpid(), eventid_arr[fork_ctr]);
		wait_ret = accevt_wait(eventid_arr[fork_ctr]);
		if (wait_ret == 0) {
			if (fork_ctr == 0)
				printf("%d detected horizontal shake\n",
							getpid());
			else if (fork_ctr == 1)
				printf("%d detected vertical shake\n",
							getpid());
			else if (fork_ctr == 2)
				printf("%d detected a shake\n", getpid());
		} else
			printf("Proc %d ended-Event destroyed.\n", getpid());
		/*Tested by sending the signals here using the signaler binary*/
		/*wait_ret = accevt_destroy(event_id);*/
	} else {
		sleep(60);
		int ctr = 0;

		for (ctr = 0; ctr < NUM_EVENTS; ctr++)
			accevt_destroy(eventid_arr[ctr]);
		exit(0);
	}
	return 0;
}
