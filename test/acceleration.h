
#ifndef _ACCELERATION_H
#define _ACCELERATION_H

struct dev_acceleration{
	int x;
	int y;
	int z;
};

struct acc_motion {
	unsigned int dlt_x; /* +/- around X-axis */
	unsigned int dlt_y; /* +/- around Y-axis */
	unsigned int dlt_z; /* +/- around Z-axis */

	unsigned int frq;   /* Number of samples that satisfies:
                          sum_each_sample(dlt_x + dlt_y + dlt_z) > NOISE */
};

#endif
