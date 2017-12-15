#include "reader.h"
#include "device.h"


#define G610_DEV_0_READ_MAX_SIZE 8
#define G610_DEV_1_READ_MAX_SIZE 64


bool gkbd_reader_create(gkbd_reader * reader, hid_device * dev, void * read_function) {
	if (reader->thread)
		return false;
	reader->thread = malloc(sizeof(pthread_t));
	if (!reader->thread)
		return false;
	reader->dev = dev;
	reader->keys_count = 0;
	pthread_mutex_init(&reader->mutex, NULL);
	if (pthread_create(reader->thread, NULL, read_function, reader)) {
		free(reader->thread);
		reader->thread = NULL;
		return false;
	}
	return true;
}

void gkbd_reader_destroy(gkbd_reader * reader) {
	if (!reader->thread)
		return;
	pthread_detach(*reader->thread);
	free(reader->thread);
	reader->thread = NULL;
}


void * gkbd_reader_read_0(void * arg) {
	gkbd_reader * reader = (gkbd_reader *) arg;
	uint8_t hid_buffer[G610_DEV_0_READ_MAX_SIZE];
	gkbd_key key_buffer[G610_DEV_0_READ_MAX_SIZE];
	size_t hid_count, keys_count;

	while (true) {
		hid_count = hid_read(reader->dev, hid_buffer, G610_DEV_0_READ_MAX_SIZE);
		keys_count = 0;

		for (int i = 0; i < 8; i++) {
			if (hid_buffer[0] & 0x01 << i)
				key_buffer[keys_count++] = G610_KEY_GROUP_MODIFIERS[i];
		}

		for (int i = 2; i < hid_count; i++) {
			if (!hid_buffer[i])
				break;
			key_buffer[keys_count++] = ADDRESS_GROUP_KEYS << 8 | hid_buffer[i];
		}

		pthread_mutex_lock(&reader->mutex);
		memcpy(reader->keys, key_buffer, keys_count * sizeof(gkbd_key));
		reader->keys_count = keys_count;
		pthread_mutex_unlock(&reader->mutex);
	}

	return NULL;
}

void * gkbd_reader_read_1(void * arg) {
	gkbd_reader * reader = (gkbd_reader *) arg;
	gkbd_device * device = (gkbd_device *) arg;

	uint8_t hid_buffer[G610_DEV_1_READ_MAX_SIZE];
	gkbd_key key_buffer[G610_DEV_1_READ_MAX_SIZE];
	size_t hid_count, keys_count;

	while (true) {
		hid_count = hid_read(reader->dev, hid_buffer, G610_DEV_1_READ_MAX_SIZE);

		if (hid_buffer[0] != 0x01)
			continue;

		keys_count = 0;
		for (int i = 1; i < hid_count - 1; i++) {
			if (!hid_buffer[i])
				break;
			key_buffer[keys_count++] = ADDRESS_GROUP_KEYS << 8 | hid_buffer[i];
		}

		pthread_mutex_lock(&reader->mutex);
		memcpy(reader->keys, key_buffer, keys_count * sizeof(gkbd_key));
		reader->keys_count = keys_count;
		pthread_mutex_unlock(&reader->mutex);
	}

	return NULL;
}

size_t gkbd_read_keys(gkbd_device * device, gkbd_key * keys, size_t length) {
	gkbd_reader * reader;
	size_t count_0 = 0;
	size_t count_1 = 0;

	if (!device->reader_0) {
		device->reader_0 = calloc(1, sizeof(gkbd_reader));
		gkbd_reader_create(device->reader_0, device->dev_0, gkbd_reader_read_0);
		if (!device->reader_0->thread) {
			gkbd_reader_destroy(device->reader_0);
			device->reader_0 = NULL;
			fprintf(stderr, "Could not initialize reader 0\n");
			return -1;
		}
	}
	if (!device->reader_1) {
		device->reader_1 = calloc(1, sizeof(gkbd_reader));
		gkbd_reader_create(device->reader_1, device->dev_1, gkbd_reader_read_1);
		if (!device->reader_1->thread) {
			gkbd_reader_destroy(device->reader_1);
			device->reader_1 = NULL;
			fprintf(stderr, "Could not initialize reader 1\n");
			return -2;
		}
	}

	reader = device->reader_0;
	pthread_mutex_lock(&reader->mutex);
	count_0 = reader->keys_count < length ? reader->keys_count : length;
	memcpy(keys, reader->keys, count_0 * sizeof(gkbd_key));
	pthread_mutex_unlock(&reader->mutex);

	reader = device->reader_1;
	pthread_mutex_lock(&reader->mutex);
	count_1 = reader->keys_count < length - count_0 ? reader->keys_count : length - count_0;
	memcpy(keys + count_0, reader->keys, count_1 * sizeof(gkbd_key));
	pthread_mutex_unlock(&reader->mutex);

	return count_0 + count_1;
}
