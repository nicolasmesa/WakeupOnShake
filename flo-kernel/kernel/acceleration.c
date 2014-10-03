#include<linux/unistd.h>
#include<linux/linkage.h>
#include<linux/kernel.h>
#include<linux/acceleration.h>
#include<linux/syscalls.h>

SYSCALL_DEFINE1(set_acceleration, struct dev_acceleration __user *, acceleration)
{
	printk(KERN_WARNING "Syscall working\n");
	return 100;
}
