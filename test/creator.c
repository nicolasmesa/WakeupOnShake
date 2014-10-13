#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "acceleration.h"


#define __NR_set_acceleration 378
#define __NR_accevt_create 379
#define __NR_accevt_wait 380
#define __NR_accevt_signal 381
#define __NR_accevt_destroy 382

#define TIME_BETWEEN_POLLS 10

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
	struct acc_motion myAcceleration;
	int dlt_x, dlt_y, dlt_z, freq, id;

	if (argc < 5) {
		printf("Usage: %s <dlt_x> <dlt_y> <dlt_z> <freq>\n", argv[0]);
		return -1;
	}

	dlt_x = atoi(argv[1]);
	dlt_y = atoi(argv[2]);
	dlt_z = atoi(argv[3]);
	freq = atoi(argv[4]);

	myAcceleration.dlt_x = dlt_x * 100;
	myAcceleration.dlt_y = dlt_y * 100;
	myAcceleration.dlt_z = dlt_z * 100;
	myAcceleration.frq = freq;

	id = accevt_create(&myAcceleration);

	if (id < 0)
		printf("Error: %s\n", strerror(errno));
	else
		printf("The returned id for the event is %d\n", id);
	return 0;
}
