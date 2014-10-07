#include <stdio.h>
#include <unistd.h>
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
	struct acc_motion myAcceleration;
	struct dev_acceleration devAcc;

	accevt_create(&myAcceleration);
	accevt_wait(10);
	accevt_signal(&devAcc);
	accevt_destroy(10);

	return 0;			
}
