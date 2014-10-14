/*
 * Columbia University
 * COMS W4118 Fall 2014
 * Homework 3
 *
 */
#include <bionic/errno.h> /* Google does things a little different...*/
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <hardware/hardware.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <hardware/sensors.h> /* <-- This is a good place to look! */
#include "../flo-kernel/include/linux/akm8975.h"
#include "acceleration.h"

/* from sensors.c */
#define ID_ACCELERATION   (0)
#define ID_MAGNETIC_FIELD (1)
#define ID_ORIENTATION	  (2)
#define ID_TEMPERATURE	  (3)

#define SENSORS_ACCELERATION   (1<<ID_ACCELERATION)
#define SENSORS_MAGNETIC_FIELD (1<<ID_MAGNETIC_FIELD)
#define SENSORS_ORIENTATION    (1<<ID_ORIENTATION)
#define SENSORS_TEMPERATURE    (1<<ID_TEMPERATURE)


#define __NR_set_acceleration 378
#define __NR_accevt_create 379
#define __NR_accevt_wait 380
#define __NR_accevt_signal 381
#define __NR_accevt_destroy 382
#define TIME_BETWEEN_POLLS 200

/* set to 1 for a bit of debug output */
#if 1
	#define dbg(fmt, ...) printf("Accelerometer: " fmt, ## __VA_ARGS__)
#else
	#define dbg(fmt, ...)
#endif

static int effective_sensor;

/* helper functions which you should use */
static int open_sensors(struct sensors_module_t **hw_module,
			struct sensors_poll_device_t **poll_device);
static void enumerate_sensors(const struct sensors_module_t *sensors);

int set_acceleration(struct dev_acceleration  *acceleration)
{
	return syscall(__NR_set_acceleration, acceleration);
}

int accevt_create(struct acc_motion *acceleration)
{
	return syscall(__NR_accevt_create, acceleration);
}

int accevt_wait(int event_id)
{
	return syscall(__NR_accevt_wait, event_id);
}

int accevt_signal(struct dev_acceleration *acceleration)
{
	return syscall(__NR_accevt_signal, acceleration);
}

int accevt_destroy(int event_id)
{
	return syscall(__NR_accevt_destroy, event_id);
}


static int poll_sensor_data(struct sensors_poll_device_t *sensors_device)
{
	const size_t numEventMax = 16;
	const size_t minBufferSize = numEventMax;
	struct dev_acceleration acceleration;
	sensors_event_t buffer[minBufferSize];

	printf("Befor sensors_dev call\n");
	ssize_t count = sensors_device->poll(sensors_device, buffer,
		minBufferSize);
	int i, ret;

	printf("Here\n");
	for (i = 0; i < count; ++i) {
		if (buffer[i].sensor != effective_sensor)
			continue;

		/* At this point we should have valid data*/
		/* Scale it and pass it to kernel */

		acceleration.x = buffer[i].acceleration.x * 100;
		acceleration.y = buffer[i].acceleration.y * 100;
		acceleration.z = buffer[i].acceleration.z * 100;

		/* ret = set_acceleration(&acceleration); */
		ret = accevt_signal(&acceleration);
		if (ret < 0)
			printf("Error setting acceleration\n");
	}
	return 0;
}

/* entry point: fill in daemon implementation
   where indicated */
int main(int argc, char **argv)
{
	effective_sensor = -1;
	struct sensors_module_t *sensors_module = NULL;
	struct sensors_poll_device_t *sensors_device = NULL;
	int pid;

	pid = fork();
	if (pid == 0) {
		if (setsid() < 0) {
			printf("Error: %s\n", strerror(errno));
			exit(1);
		}

		if (chdir("/") < 0) {
			printf("Error: %s\n", strerror(errno));
			exit(1);
		}

		if (close(0) < 0) {
			printf("Error: %s\n", strerror(errno));
			exit(1);
		}

		if (close(1) < 0) {
			printf("Error: %s\n", strerror(errno));
			exit(1);
		}

		if (close(2) < 0) {
			printf("Error: %s\n", strerror(errno));
			exit(1);
		}
	} else if (pid > 0) {
		exit(0);
	} else {
		printf("Error: %s\n", strerror(errno));
		exit(errno);
	}


	printf("Opening sensors...\n");
	if (open_sensors(&sensors_module,
			 &sensors_device) < 0) {
		printf("open_sensors failed\n");
		return EXIT_FAILURE;
	}
	enumerate_sensors(sensors_module);


	/* Fill in daemon implementation around here */
	printf("turn me into a daemon!\n");
	while (1) {
		poll_sensor_data(sensors_device);
		usleep(TIME_BETWEEN_POLLS);
	}

	return EXIT_SUCCESS;
}

/*                DO NOT MODIFY BELOW THIS LINE                    */
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static int open_sensors(struct sensors_module_t **mSensorModule,
			struct sensors_poll_device_t **mSensorDevice)
{
	int err = hw_get_module(SENSORS_HARDWARE_MODULE_ID,
		(hw_module_t const **)mSensorModule);

	if (err) {
		printf("couldn't load %s module (%s)",
			SENSORS_HARDWARE_MODULE_ID, strerror(-err));
	}

	if (!*mSensorModule)
		return -1;

	err = sensors_open(&((*mSensorModule)->common), mSensorDevice);

	if (err) {
		printf("couldn't open device for module %s (%s)",
			SENSORS_HARDWARE_MODULE_ID, strerror(-err));
	}

	if (!*mSensorDevice)
		return -1;

	const struct sensor_t *list;
	ssize_t count = (*mSensorModule)->get_sensors_list
				(*mSensorModule, &list);
	size_t i;

	for (i = 0 ; i < (size_t)count ; i++) {
		(*mSensorDevice)->setDelay(*mSensorDevice, list[i].handle,
			TIME_BETWEEN_POLLS * 1000000);
		(*mSensorDevice)->activate(*mSensorDevice, list[i].handle, 1);
	}

	return 0;
}

static void enumerate_sensors(const struct sensors_module_t *sensors)
{
	int nr, s;
	const struct sensor_t *slist = NULL;

	if (!sensors)
		printf("going to fail\n");
	nr = sensors->get_sensors_list((struct sensors_module_t *)sensors,
					&slist);
	if (nr < 1 || slist == NULL) {
		printf("no sensors!\n");
		return;
	}

	for (s = 0; s < nr; s++) {
		printf("%s (%s) v%d\n\tHandle:%d, type:%d, max:%0.2f, "
			"resolution:%0.2f \n", slist[s].name, slist[s].vendor,
			slist[s].version, slist[s].handle, slist[s].type,
			slist[s].maxRange, slist[s].resolution);

		/* Awful hack to make it work on emulator */
		if (slist[s].type == 1 && slist[s].handle == 0)
			effective_sensor = 0; /*the sensor ID*/
	}
}
