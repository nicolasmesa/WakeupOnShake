#ifndef _DEV_ACCELERATION_H
#define _DEV_ACCELERATION_H

struct dev_acceleration {
	int x; /* acceleration along X-axis */
	int y; /* acceleration along Y-axis */
	int z; /* acceleration along Z-axis */
};

extern struct dev_acceleration curr_acceleration;
#endif
