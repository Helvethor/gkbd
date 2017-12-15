#include "writer.h"
#include "device.h"

#define G610_DEV_0_WRITE_MAX_SIZE 65

static const uint8_t G610_COMMIT[8] = { 0x00, 0x11, 0xff, 0x0c, 0x5a };

static const uint8_t G610_GROUP_ADDRESS[][8] = {
	{ 0x11, 0xff, 0x0c, 0x3a, 0x00, 0x10, 0x00, 0x01 }, // GROUP LOGO
	{ 0x12, 0xff, 0x0c, 0x3a, 0x00, 0x40, 0x00, 0x05 }, // GROUP INDICATORS
	{ 0x12, 0xff, 0x0c, 0x3a, 0x00, 0x02, 0x00, 0x05 }, // GROUP MULTIMEDIA
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // GROUP GKEYS
	{ 0x12, 0xff, 0x0c, 0x3a, 0x00, 0x01, 0x00, 0x0e }  // GROUP KEYS
};


g610_address_group g610_get_address_group(g610_key key) {
	return key >> 8;
}

size_t g610_address_group_addrcpy(g610_address_group address_group, uint8_t * buffer, size_t length) {
	if (address_group == ADDRESS_GROUP_GKEYS)
		return 0;
	if (length < 8)
		return -1;
	memcpy(buffer, G610_GROUP_ADDRESS[address_group], 8);
	return 8;
}

size_t g610_address_group_message_size(g610_address_group address_group) {
	if (address_group == ADDRESS_GROUP_LOGO)
		return 21;
	return G610_DEV_0_WRITE_MAX_SIZE;
}


g610_led_buffer g610_led_buffer_create(g610_address_group address_group) {
	g610_led_buffer buffer = {
		.address_group = address_group,
		.leds_count = 0
	};

	if (address_group == ADDRESS_GROUP_LOGO)
		buffer.leds_length = 5;
	else
		buffer.leds_length = 14;
	
	buffer.leds = malloc(buffer.leds_length * sizeof(g610_led));
	if (buffer.leds == NULL)
		buffer.leds_length = 0;

	return buffer;
}

void g610_led_buffer_destroy(g610_led_buffer * buffer) {
	if (buffer->leds != NULL)
		free(buffer->leds);
	buffer->leds_length = 0;
}

bool g610_commit(g610_device * g610_device) {
	if (hid_write(g610_device->dev_0, G610_COMMIT, sizeof(G610_COMMIT) / sizeof(uint8_t)) < 0)
		return false;
	return true;
}

bool g610_write_led_buffer(g610_device * device, g610_led_buffer * led_buffer) {
	bool result = true;
	int res;
	size_t length = 1;
	uint8_t buffer[G610_DEV_0_WRITE_MAX_SIZE] = { 0x00 };

	if (led_buffer->leds_count <= 0)
		return true;

	length += g610_address_group_addrcpy(led_buffer->address_group,
		buffer + length, G610_DEV_0_WRITE_MAX_SIZE - length);

	for (int i = 0; i < led_buffer->leds_count; i++) {
		buffer[length++] = (uint8_t) led_buffer->leds[i].key;
		buffer[length++] = led_buffer->leds[i].intensity;
		buffer[length++] = led_buffer->leds[i].intensity;
		buffer[length++] = led_buffer->leds[i].intensity;
	}

	if ((res = hid_write(device->dev_0, buffer,
		g610_address_group_message_size(led_buffer->address_group))) < 0)
		return false;
	return true;
}

bool g610_write_leds(g610_device * device, g610_led * leds, size_t length) {
	if (length < 1) return true;
	bool result = true;
	g610_led_buffer logo_buffer = g610_led_buffer_create(ADDRESS_GROUP_LOGO);
	g610_led_buffer indicators_buffer = g610_led_buffer_create(ADDRESS_GROUP_INDICATORS);
	g610_led_buffer multimedia_buffer = g610_led_buffer_create(ADDRESS_GROUP_MULTIMEDIA);
	g610_led_buffer gkeys_buffer = g610_led_buffer_create(ADDRESS_GROUP_GKEYS);
	g610_led_buffer keys_buffer = g610_led_buffer_create(ADDRESS_GROUP_KEYS);

	g610_led_buffer * buffer;
	g610_led remaining_leds[length];
	size_t remaining_count = 0;

	for (int i = 0; i < length; i++) {

		switch (g610_get_address_group(leds[i].key)) {
		case ADDRESS_GROUP_LOGO:
			buffer = &logo_buffer;
			break;
		case ADDRESS_GROUP_INDICATORS:
			buffer = &indicators_buffer;
			break;
		case ADDRESS_GROUP_MULTIMEDIA:
			buffer = &multimedia_buffer;
			break;
		case ADDRESS_GROUP_GKEYS:
			buffer = &gkeys_buffer;
			break;
		case ADDRESS_GROUP_KEYS:
			buffer = &keys_buffer;
			break;
		}

		if (buffer->leds_count >= buffer->leds_length || buffer->leds == NULL) {
			remaining_leds[remaining_count++] = leds[i];
		}
		else {
			buffer->leds[buffer->leds_count++] = leds[i];
		}
	}

	result &= g610_write_led_buffer(device, &logo_buffer);
	result &= g610_write_led_buffer(device, &indicators_buffer);
	result &= g610_write_led_buffer(device, &multimedia_buffer);
	result &= g610_write_led_buffer(device, &gkeys_buffer);
	result &= g610_write_led_buffer(device, &keys_buffer);
	result &= g610_commit(device);

	g610_led_buffer_destroy(&logo_buffer);
	g610_led_buffer_destroy(&indicators_buffer);
	g610_led_buffer_destroy(&multimedia_buffer);
	g610_led_buffer_destroy(&gkeys_buffer);
	g610_led_buffer_destroy(&keys_buffer);

	g610_write_leds(device, remaining_leds, remaining_count);

	return result;
}

bool g610_write_led(g610_device * g610_device, g610_led led) {
	return g610_write_leds(g610_device, &led, 1);
}


