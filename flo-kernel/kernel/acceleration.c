#include <linux/unistd.h>
#include <linux/sched.h>
#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/acceleration.h>

struct dev_acceleration curr_acceleration;
SYSCALL_DEFINE1(set_acceleration, struct dev_acceleration __user *, acceleration)
{
	if (current->cred->uid != 0) {
		printk(KERN_CRIT "Non root user trying to set acceleration: %d\n", current->cred->uid);
		return -EACCES;
	}

	printk(KERN_CRIT "Kernel previous accelerations:\n\t- x: %d\n\t- y: %d\n\t- z: %d", curr_acceleration.x, curr_acceleration.y, curr_acceleration.z);

	if (acceleration == NULL) {
		printk(KERN_WARNING "Error: acceleration is NULL\n");
		return -EINVAL;
	}

	if (copy_from_user(&curr_acceleration, acceleration, sizeof(curr_acceleration))) {
		printk(KERN_WARNING "Error while copying acceleration\n");
		return -EFAULT;
	}

	printk(KERN_CRIT "Kernel received accelerations:\n\t- x: %d\n\t- y: %d\n\t- z: %d", curr_acceleration.x, curr_acceleration.y, curr_acceleration.z);

	return 0;
}

SYSCALL_DEFINE1(accevt_create, struct acc_motion __user *, acceleration)
{
	printk(KERN_WARNING "accevt called\n");
	return 0;
}


SYSCALL_DEFINE1(accevt_wait, int, event_id)
{
	printk(KERN_WARNING "accevt_wait called\n");
	return 0;
}


SYSCALL_DEFINE1(accevt_signal, struct dev_acceleration __user *, acceleration)
{
	printk(KERN_WARNING "accevt_signal called\n");
	return 0;
}

SYSCALL_DEFINE1(accevt_destroy, int, event_id)
{
	printk(KERN_WARNING "accevt_destroy called\n");
	return 0;
}

