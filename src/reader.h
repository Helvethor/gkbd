#pragma once

#include <pthread.h>
#include "g610.h"


typedef struct g610_reader {
	hid_device * dev;
	g610_key keys[G610_READ_MAX_SIZE];
	size_t keys_count;
	pthread_t * thread;
	pthread_mutex_t mutex;
} g610_reader;

void g610_reader_destroy(g610_reader * reader);
