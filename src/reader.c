#include "reader.h"
#include "device.h"


#define G610_DEV_0_READ_MAX_SIZE 8
#define G610_DEV_1_READ_MAX_SIZE 64


bool g610_create_reader(g610_reader * reader, hid_device * dev, void * read_function) {
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

void g610_reader_destroy(g610_reader * reader) {
	if (!reader->thread)
		return;
	pthread_detach(*reader->thread);
	free(reader->thread);
	reader->thread = NULL;
}


void * g610_reader_read_0(void * arg) {
	g610_reader * reader = (g610_reader *) arg;
	uint8_t hid_buffer[G610_DEV_0_READ_MAX_SIZE];
	g610_key key_buffer[G610_DEV_0_READ_MAX_SIZE];
	size_t hid_count, keys_count;

	while (true) {
		hid_count = hid_read(reader->dev, hid_buffer, G610_DEV_0_READ_MAX_SIZE);
		keys_count = 0;

		for (int i = 0; i < 8; i++) {
			if (hid_buffer[0] & 0x01 << i)
				key_buffer[keys_count++] = G610_KEY_GROUP_MODIFIERS[i];
		}

		for (int i = 2; i < hid_count; i++) {
			key_buffer[keys_count++] = ADDRESS_GROUP_KEYS << 8 | hid_buffer[i];
		}

		pthread_mutex_lock(&reader->mutex);
		memcpy(reader->keys, key_buffer, keys_count * sizeof(g610_key));
		reader->keys_count = keys_count;
		pthread_mutex_unlock(&reader->mutex);
	}

	return NULL;
}

void * g610_reader_read_1(void * arg) {
	g610_reader * reader = (g610_reader *) arg;
	g610_device * device = (g610_device *) arg;

	uint8_t hid_buffer[G610_DEV_1_READ_MAX_SIZE];
	g610_key key_buffer[G610_DEV_1_READ_MAX_SIZE];
	size_t hid_count, keys_count;

	while (true) {
		hid_count = hid_read(reader->dev, hid_buffer, G610_DEV_1_READ_MAX_SIZE);
		keys_count = 0;

		if (hid_buffer[0] != 0x01)
			continue;

		for (int i = 1; i < hid_count - 1; i++) {
			key_buffer[keys_count++] = ADDRESS_GROUP_KEYS << 8 | hid_buffer[i];
		}

		pthread_mutex_lock(&reader->mutex);
		memcpy(reader->keys, key_buffer, keys_count * sizeof(g610_key));
		reader->keys_count = keys_count;
		pthread_mutex_unlock(&reader->mutex);
	}

	return NULL;
}

size_t g610_get_keys(g610_device * device, g610_key * keys, size_t length) {
	g610_reader * reader;
	size_t count_0 = 0;
	size_t count_1 = 0;

	if (!device->reader_0) {
		device->reader_0 = calloc(1, sizeof(g610_reader));
		g610_create_reader(device->reader_0, device->dev_0, g610_reader_read_0);
		if (!device->reader_0->thread) {
			g610_reader_destroy(device->reader_0);
			device->reader_0 = NULL;
			fprintf(stderr, "Could not initialize reader 0\n");
			return -1;
		}
	}
	if (!device->reader_1) {
		device->reader_1 = calloc(1, sizeof(g610_reader));
		g610_create_reader(device->reader_1, device->dev_1, g610_reader_read_1);
		if (!device->reader_1->thread) {
			g610_reader_destroy(device->reader_1);
			device->reader_1 = NULL;
			fprintf(stderr, "Could not initialize reader 1\n");
			return -2;
		}
	}

	reader = device->reader_0;
	pthread_mutex_lock(&reader->mutex);
	count_0 = reader->keys_count < length ? reader->keys_count : length;
	memcpy(keys, reader->keys, count_0 * sizeof(g610_key));
	pthread_mutex_unlock(&reader->mutex);

	reader = device->reader_1;
	pthread_mutex_lock(&reader->mutex);
	count_1 = reader->keys_count < length - count_0 ? reader->keys_count : length - count_0;
	memcpy(keys + count_0, reader->keys, count_1 * sizeof(g610_key));
	pthread_mutex_unlock(&reader->mutex);

	return count_0 + count_1;
}
