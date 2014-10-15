#include <stdio.h>
#include <unistd.h>
#include "acceleration.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

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
	int event_id, ret;

	if (argc < 2) {
		printf("Usage: %s <event_id>\n", argv[0]);
		return -1;
	}

	event_id = atoi(argv[1]);

	ret = accevt_wait(event_id);

	if (ret < 0)
		printf("Error: %s\n", strerror(errno));

	return 0;
}
