#include <stdio.h>
#include <unistd.h>
#include "acceleration.h"
#include <errno.h>

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

int main(int argc, char **argv)
{
	struct dev_acceleration devAcc;
	int x, y, z;

	if (argc < 4) {
		printf("Usage: %s <x> <y> <z>\n", argv[0]);
		return -1;
	}

	x = atoi(argv[1]);
	y = atoi(argv[2]);
	z = atoi(argv[3]);

	devAcc.x = x * 100;
	devAcc.y = y * 100;
	devAcc.z = z * 100;

	if (set_acceleration(&devAcc))
		printf("Error: %s\n", strerror(errno));

	return 0;
}
