#include <linux/unistd.h>
#include <linux/sched.h>
#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/acceleration.h>



void add_delta_acceleration(struct dev_acceleration *curr_acceleration, struct dev_acceleration * prev_acceleration)
{
	int dlt_x, dlt_y, dlt_z;

	dlt_x = curr_acceleration->x - prev_acceleration->x;
	dlt_y = curr_acceleration->y - prev_acceleration->y;
	dlt_z = curr_acceleration->z - prev_acceleration->z;

	printk(KERN_CRIT "Deltas:\tx: %d\ty: %d\tz: %d\n", dlt_x, dlt_y, dlt_x);

}

SYSCALL_DEFINE1(set_acceleration, struct dev_acceleration __user *, acceleration)
{
	static struct dev_acceleration *curr_acceleration = NULL, *prev_acceleration = NULL, *temp;
	static int first_time = 1;

	if (curr_acceleration == NULL) {
		curr_acceleration = kmalloc(sizeof(*curr_acceleration), GFP_KERNEL);
	}

	if (prev_acceleration == NULL) {
		prev_acceleration = kmalloc(sizeof(*prev_acceleration), GFP_KERNEL);
	}

	if (current->cred->uid != 0) {
		printk(KERN_CRIT "Non root user trying to set acceleration: %d\n", current->cred->uid);
		return -EACCES;
	}

	if (acceleration == NULL) {
		printk(KERN_WARNING "Error: acceleration is NULL\n");
		return -EINVAL;
	}

	if (copy_from_user(curr_acceleration, acceleration, sizeof(*curr_acceleration))) {
		printk(KERN_WARNING "Error while copying acceleration\n");
		return -EFAULT;
	}


	if (!first_time) {
		add_delta_acceleration(curr_acceleration, prev_acceleration);
	}

	/* Switch them. curr_acceleration will be overwritten in next call. Not sure if
	 this counts as a memory leak since memory is never freed */
	temp = curr_acceleration;
	curr_acceleration = prev_acceleration;
	prev_acceleration = temp;

	first_time = 0;
	return 0;
}

SYSCALL_DEFINE1(accevt_create, struct acc_motion __user *, acceleration)
{
        return 0;
}


SYSCALL_DEFINE1(accevt_wait, int, event_id)
{
	printk(KERN_WARNING "accevt_wait called\n");
	return 0;
}


SYSCALL_DEFINE1(accevt_signal, struct dev_acceleration __user *, acceleration)
{
	static struct dev_acceleration *curr_acceleration = NULL, *prev_acceleration = NULL, *temp;
        static int first_time = 1;

        if (curr_acceleration == NULL) {
                curr_acceleration = kmalloc(sizeof(*curr_acceleration), GFP_KERNEL);
        }

        if (prev_acceleration == NULL) {
                prev_acceleration = kmalloc(sizeof(*prev_acceleration), GFP_KERNEL);
        }

        if (current->cred->uid != 0) {
                printk(KERN_CRIT "Non root user trying to set acceleration: %d\n", current->cred->uid);
                return -EACCES;
        }

        if (acceleration == NULL) {
                printk(KERN_WARNING "Error: acceleration is NULL\n");
                return -EINVAL;
        }

        if (copy_from_user(curr_acceleration, acceleration, sizeof(*curr_acceleration))) {
                printk(KERN_WARNING "Error while copying acceleration\n");
                return -EFAULT;
        }


        if (!first_time) {
                add_delta_acceleration(curr_acceleration, prev_acceleration);
        }

        /* Switch them. curr_acceleration will be overwritten in next call. Not sure if
         this counts as a memory leak since memory is never freed */
        temp = curr_acceleration;
        curr_acceleration = prev_acceleration;
        prev_acceleration = temp;

        first_time = 0;
        return 0;
}

SYSCALL_DEFINE1(accevt_destroy, int, event_id)
{
	printk(KERN_WARNING "accevt_destroy called\n");
	return 0;
}

