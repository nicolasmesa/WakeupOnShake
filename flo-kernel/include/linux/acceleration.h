#ifndef _DEV_ACCELERATION_H
#define _DEV_ACCELERATION_H

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

extern struct dev_acceleration curr_acceleration;
#endif
