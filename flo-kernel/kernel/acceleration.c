#include <linux/unistd.h>
#include <linux/sched.h>
#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/acceleration.h>
#include <linux/spinlock.h>

static struct context evtCtx;

static struct deltas buffer[WINDOW];
static int acc_count = 0;
DEFINE_SPINLOCK(buffer_lock);
DEFINE_SPINLOCK(events_lock);

void add_delta_acceleration(struct dev_acceleration *curr_acceleration, struct dev_acceleration * prev_acceleration)
{

	spin_lock(&buffer_lock);

	buffer[acc_count % WINDOW].dlt_x = curr_acceleration->x - prev_acceleration->x;
	buffer[acc_count % WINDOW].dlt_y = curr_acceleration->y - prev_acceleration->y;
	buffer[acc_count % WINDOW].dlt_z = curr_acceleration->z - prev_acceleration->z;

	if (buffer[acc_count % WINDOW].dlt_x < 0)
		buffer[acc_count % WINDOW].dlt_x *= -1;

	if (buffer[acc_count % WINDOW].dlt_y < 0)
		buffer[acc_count % WINDOW].dlt_y *= -1;

	if (buffer[acc_count % WINDOW].dlt_z < 0)
		buffer[acc_count % WINDOW].dlt_z *= -1;

	acc_count++;

	spin_unlock(&buffer_lock);
}

int check_noise(struct deltas *delta)
{
	return (delta->dlt_x + delta->dlt_y + delta->dlt_z) > NOISE;
}

int delta_is_relevant(struct deltas *deltas, struct acc_motion *event)
{
	if (deltas->dlt_x < event->dlt_x)
		return 0;

	if (deltas->dlt_y < event->dlt_y)
		return 0;

	if (deltas->dlt_z < event->dlt_z)
		return 0;

	return 1;
}

void search_and_signal(void)
{
	struct motion_event *event;
	int freq, i, count;

	spin_lock(&buffer_lock);
	spin_lock(&events_lock);

	count = acc_count > WINDOW ? WINDOW : acc_count;

	list_for_each_entry(event, &(evtCtx.events), events) {
		freq = 0;
		printk("Analysis of event %d:\t%d\t%d\t%d\n", event->id, event->motion.dlt_x, event->motion.dlt_y, event->motion.dlt_z);
		for (i = 0; i < count; i++) {
			printk("Checking buffer[%d]\t%d\t%d\t%d\n", i, buffer[i].dlt_x, buffer[i].dlt_y, buffer[i].dlt_z);
			if (!check_noise(&(buffer[i])))
				continue;

			printk("Noise check passed\n");

			if (!delta_is_relevant(&(buffer[i]), &event->motion))
				continue;

			freq++;

			printk("Delta is relevant:\tCurr_freq: %d\tDesired_fre: %d\n", freq, event->motion.frq);

			if (freq >= event->motion.frq) {
				printk("Wake up event id: %d\n", event->id);
				break;
			}
		}
	}


	spin_unlock(&events_lock);
	spin_unlock(&buffer_lock);
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
	struct motion_event *new_event;
	static int first_time = 1;
	int id;

	new_event = kmalloc(sizeof(*new_event),  GFP_KERNEL);
       
	if (new_event == NULL)
		return -ENOMEM;

	spin_lock(&events_lock);

	if (first_time) {
		first_time = 0;
		evtCtx.current_id = 1;
		INIT_LIST_HEAD(&(evtCtx.events));
	}


	if (acceleration == NULL) {
		spin_unlock(&events_lock);
		kfree(new_event);
		return -EINVAL;
	}

	if (copy_from_user(&new_event->motion, acceleration, sizeof(struct acc_motion))) {
		spin_unlock(&events_lock);
		kfree(new_event);
		return -EFAULT;
	}


	id = evtCtx.current_id++;
	new_event->id = id;

	INIT_LIST_HEAD(&new_event->events);
	new_event->deletedFlag = 0;
	new_event->referenceCount = 0;

	list_add(&new_event->events, &(evtCtx.events));

	spin_unlock(&events_lock);

        return id;
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
		search_and_signal();
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

