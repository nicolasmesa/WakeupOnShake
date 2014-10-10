#include <linux/unistd.h>
#include <linux/sched.h>
#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/acceleration.h>
#include <linux/spinlock.h>
#include <linux/wait.h>

static struct context evtCtx = {
	.current_id = 1,
	.events = LIST_HEAD_INIT(evtCtx.events),
};

static struct deltas buffer[WINDOW];
static int acc_count = 0;
DEFINE_SPINLOCK(buffer_lock);
DEFINE_SPINLOCK(events_lock);

int search_and_add(int event_id)
{
	int ret = 0, found = 0, my_wake_up_counter, deleted = 0;
	struct motion_event *event, *temp;
	DEFINE_WAIT(wait);

	spin_lock(&events_lock);


	list_for_each_entry_safe(event, temp, &(evtCtx.events), events) {
		if (event->id == event_id) {
			found = 1;
			event->referenceCount++;
			my_wake_up_counter = event->wake_up_counter;
			add_wait_queue(&event->queue, &wait);

			while (my_wake_up_counter == event->wake_up_counter) {
				prepare_to_wait(&event->queue, &wait, TASK_INTERRUPTIBLE);

				if (signal_pending(current)){
					event->referenceCount--;
					finish_wait(&event->queue, &wait);
					spin_unlock(&events_lock);
					return -1;
				}

				spin_unlock(&events_lock);
				schedule();
				spin_lock(&events_lock);
			}

			finish_wait(&event->queue, &wait);	

			event->referenceCount--;

			if (event->deletedFlag) {
				deleted = 1;
			}

			if (deleted && event->referenceCount <= 0) {
				list_del(&event->events);
				kfree(event);
			}

			break;
		}
	}

	spin_unlock(&events_lock);

	ret = deleted ? -2 : found;

	return found;
}

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
		for (i = 0; i < count; i++) {
			if (!check_noise(&(buffer[i])))
				continue;

			if (!delta_is_relevant(&(buffer[i]), &event->motion))
				continue;

			freq++;

			if (freq >= event->motion.frq) {
				event->wake_up_counter++;
				wake_up_all(&event->queue);
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
	int id;

	new_event = kmalloc(sizeof(*new_event),  GFP_KERNEL);
       
	if (new_event == NULL)
		return -ENOMEM;

	spin_lock(&events_lock);

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

	if (new_event->motion.frq > WINDOW) 
		new_event->motion.frq = WINDOW;

	INIT_LIST_HEAD(&new_event->events);
	init_waitqueue_head(&new_event->queue);
	new_event->deletedFlag = 0;
	new_event->referenceCount = 0;
	new_event->wake_up_counter = 0;

	list_add(&new_event->events, &(evtCtx.events));

	spin_unlock(&events_lock);

        return id;
}


SYSCALL_DEFINE1(accevt_wait, int, event_id)
{

	if (event_id < 1)
		return -EINVAL;

	if (search_and_add(event_id))
		return 0;
	else
		return -EINVAL;
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
	struct motion_event *event, *temp;
	int found = 0;

	spin_lock(&events_lock);

	list_for_each_entry_safe(event, temp, &(evtCtx.events), events) {
		if (event->id == event_id) {
			found = 1;
			event->deletedFlag = 1;

			if (event->referenceCount == 0) {
				list_del(&event->events);
				kfree(event);
			} else {
				event->wake_up_counter++;
				wake_up_all(&event->queue);
			}

			break;	
		}
	}

	spin_unlock(&events_lock);

	return 0;
}

