#pragma once

#include <pthread.h>
#include "gkbd.h"


typedef struct gkbd_reader {
	hid_device * dev;
	gkbd_key keys[G610_READ_MAX_SIZE];
	size_t keys_count;
	pthread_t * thread;
	pthread_mutex_t mutex;
} gkbd_reader;

void gkbd_reader_destroy(gkbd_reader * reader);
