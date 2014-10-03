#include<linux/unistd.h>
#include<linux/sched.h>
#include<linux/linkage.h>
#include<linux/kernel.h>
#include<linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include<linux/acceleration.h>

SYSCALL_DEFINE1(set_acceleration, struct dev_acceleration __user *, acceleration)
{
	struct dev_acceleration k_acceleration;

	if (acceleration == NULL) {
		printk(KERN_WARNING "Error: acceleration is NULL\n");
		return -EINVAL;
	}

	if (copy_from_user(&k_acceleration, acceleration, sizeof(k_acceleration))) {
		printk(KERN_WARNING "Error while copying acceleration\n");
		return -EFAULT;
	}

	printk(KERN_WARNING "Kernel received accelerations:\n\t- x: %d\n\t- y: %d\n\t- z: %d", k_acceleration.x, k_acceleration.y, k_acceleration.z);

	return 0;
}
