#ifndef _DEV_ACCELERATION_H
#define _DEV_ACCELERATION_H

#include <linux/list.h>
#include <linux/wait.h>

struct dev_acceleration {
	int x; /* acceleration along X-axis */
	int y; /* acceleration along Y-axis */
	int z; /* acceleration along Z-axis */
};


/*Define the noise*/
#define NOISE 10

/*Define the window*/
#define WINDOW 20

/*
* Define the motion.
* The motion give the baseline for an EVENT.
*/
struct acc_motion {

     unsigned int dlt_x; /* +/- around X-axis */
     unsigned int dlt_y; /* +/- around Y-axis */
     unsigned int dlt_z; /* +/- around Z-axis */
     
     unsigned int frq;   /* Number of samples that satisfies:
                          sum_each_sample(dlt_x + dlt_y + dlt_z) > NOISE */
};

struct motion_event {
	int id;
	struct acc_motion motion;
	int referenceCount;
	int deletedFlag;
	wait_queue_head_t queue;
	struct list_head events;
};


struct context {
	int current_id;
	struct list_head events;
};

struct deltas {
	unsigned int dlt_x;
	unsigned int dlt_y;
	unsigned int dlt_z;
};

#endif
