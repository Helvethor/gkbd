#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "hidapi/hidapi.h"
#include "g610.h"


#define EVENTS_MAX_SIZE 128
#define NANOSEC_PER_SEC 1000000000

struct timespec timespec_diff(struct timespec * a, struct timespec * b) {
	struct timespec c;
	c.tv_sec = a->tv_sec - b->tv_sec;
	c.tv_nsec = a->tv_nsec - b->tv_nsec;
	while (c.tv_nsec < 0) {
		c.tv_sec -= 1;
		c.tv_nsec += NANOSEC_PER_SEC;
	}
	
	return c;
}

bool timespec_gt(struct timespec * a, struct timespec * b) {
	if (a->tv_sec < b->tv_sec)
		return false;
	if (a->tv_sec > b->tv_sec)
		return true;
	return a->tv_nsec > b->tv_nsec;
}

typedef struct led_event {
	g610_led led;
	bool active;
	uint8_t decrement;
	struct timespec last_clock;
	struct timespec step_duration;
} led_event;


typedef struct led_event_buffer {
	led_event * events;
	size_t count;
	size_t length;
} led_event_buffer;


led_event_buffer * led_event_buffer_create(size_t length) {

	led_event_buffer * buffer = malloc(sizeof(led_event_buffer));
	if (!buffer)
		return NULL;

	buffer->events = malloc(sizeof(led_event) * length);
	if (!buffer->events) {
		free(buffer);
		return NULL;
	}

	buffer->length = length;
	buffer->count = 0;

	return buffer;
}

void led_event_buffer_destroy(led_event_buffer * buffer) {
	free(buffer->events);
	free(buffer);
}


led_event * led_event_find_event(led_event_buffer * buffer, g610_key key) {
	led_event * event = NULL;
	size_t i;

	for (i = 0; i < buffer->count; i++) {
		if (buffer->events[i].led.key == key) {
			event = buffer->events + i;
			break;
		}
		else if (!buffer->events[i].active)
			event = buffer->events + i;
	}

	if (!event && buffer->count < buffer->length)
		event = buffer->events + buffer->count++;

	return event;
}

bool led_event_set_led(led_event_buffer * buffer, g610_led led,
	uint8_t decrement, struct timespec step_duration) {
	struct timespec now;

	led_event * event = led_event_find_event(buffer, led.key);
	if (!event)
		return false;

	clock_gettime(CLOCK_MONOTONIC, &now);
	event->led = led;
	event->decrement = decrement;
	event->step_duration = step_duration;
	event->last_clock = timespec_diff(&now, &step_duration);
	event->active = true;

	return true;
}


bool led_event_update(led_event_buffer * buffer, g610_device * device) {
	size_t i, leds_count = 0;
	led_event * event;
	struct timespec now, elapsed_time;
	g610_led * led;
	g610_led leds[buffer->count];

	for (i = 0; i < buffer->count; i++) {
		event = buffer->events + i;
		clock_gettime(CLOCK_MONOTONIC, &now);
		elapsed_time = timespec_diff(&now, &event->last_clock);

		if (!event->active)
			continue;
		if (timespec_gt(&event->step_duration, &elapsed_time))
			continue;
		elapsed_time = timespec_diff(&elapsed_time, &event->step_duration);
		
		leds[leds_count++] = event->led;
		led = &event->led;

		if (led->intensity == 0x00)
			event->active = false;
		else {
			if (led->intensity <= event->decrement)
				led->intensity = 0x00;
			else
				led->intensity -= event->decrement;
			clock_gettime(CLOCK_MONOTONIC, &event->last_clock);
		}
	}

	return g610_write_leds(device, leds, leds_count);
}


int main(int argc, char* argv[]) {
	int res, i;
	size_t event_count = 0;
	struct hid_device_info *devs, *dev;
	struct timespec step_duration, work_duration, sleep_duration;
	struct timespec update_duration, update_start, update_end;
	g610_key keys[G610_READ_MAX_SIZE];
	g610_led led;
	g610_device * g610;
	led_event_buffer * events_buffer;

	res = hid_init();
	if (res < 0) {
		fprintf(stderr, "Could not init hidapi: %d\n", res);
		return -1;
	}

	g610 = g610_open();
	if (!g610) {
		fprintf(stderr, "Couldn't open g610\n");
		return -1;
	}
	
	events_buffer = led_event_buffer_create(EVENTS_MAX_SIZE);
	if (!events_buffer) {
		fprintf(stderr, "Could not allocate led_event_buffer\n");
		return -1;
	}

	update_duration.tv_sec = 0;
	update_duration.tv_nsec = NANOSEC_PER_SEC / 1000;
	led.intensity = 0xff;

	while (true) {

		clock_gettime(CLOCK_MONOTONIC, &update_start);
		res = g610_read_keys(g610, keys, G610_READ_MAX_SIZE);

		step_duration.tv_sec = 0;
		step_duration.tv_nsec = NANOSEC_PER_SEC / 100;
		for (int i = 0; i < res; i++) {
			led.key = keys[i];
			led_event_set_led(events_buffer, led, 0x08, step_duration);
		}

		step_duration.tv_sec = 0;
		step_duration.tv_nsec = 0;
		if (res > 0) {
			led.key = KEY_LOGO;
			led_event_set_led(events_buffer, led, 0xff, step_duration);
		}

		led_event_update(events_buffer, g610);
		clock_gettime(CLOCK_MONOTONIC, &update_end);
		work_duration = timespec_diff(&update_end, &update_start);
		sleep_duration = timespec_diff(&update_duration, &work_duration);

		if (sleep_duration.tv_sec < 0)
			continue;
		usleep(sleep_duration.tv_nsec / 1000);
	}

	led_event_buffer_destroy(events_buffer);
	g610_close(g610);
	hid_exit();

	return 0;
}
