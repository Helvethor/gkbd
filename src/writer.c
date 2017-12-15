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


gkbd_address_group gkbd_get_address_group(gkbd_key key) {
	return key >> 8;
}

size_t gkbd_address_group_addrcpy(gkbd_address_group address_group, uint8_t * buffer, size_t length) {
	if (address_group == ADDRESS_GROUP_GKEYS)
		return 0;
	if (length < 8)
		return -1;
	memcpy(buffer, G610_GROUP_ADDRESS[address_group], 8);
	return 8;
}

size_t gkbd_address_group_message_size(gkbd_address_group address_group) {
	if (address_group == ADDRESS_GROUP_LOGO)
		return 21;
	return G610_DEV_0_WRITE_MAX_SIZE;
}


gkbd_led_buffer gkbd_led_buffer_create(gkbd_address_group address_group) {
	gkbd_led_buffer buffer = {
		.address_group = address_group,
		.leds_count = 0
	};

	if (address_group == ADDRESS_GROUP_LOGO)
		buffer.leds_length = 5;
	else
		buffer.leds_length = 14;
	
	buffer.leds = malloc(buffer.leds_length * sizeof(gkbd_led));
	if (buffer.leds == NULL)
		buffer.leds_length = 0;

	return buffer;
}

void gkbd_led_buffer_destroy(gkbd_led_buffer * buffer) {
	if (buffer->leds != NULL)
		free(buffer->leds);
	buffer->leds_length = 0;
}

bool gkbd_commit(gkbd_device * gkbd_device) {
	if (hid_write(gkbd_device->dev_0, G610_COMMIT, sizeof(G610_COMMIT) / sizeof(uint8_t)) < 0)
		return false;
	return true;
}

bool gkbd_write_led_buffer(gkbd_device * device, gkbd_led_buffer * led_buffer) {
	bool result = true;
	int res;
	size_t length = 1;
	uint8_t buffer[G610_DEV_0_WRITE_MAX_SIZE] = { 0x00 };

	if (led_buffer->leds_count <= 0)
		return true;

	length += gkbd_address_group_addrcpy(led_buffer->address_group,
		buffer + length, G610_DEV_0_WRITE_MAX_SIZE - length);

	for (int i = 0; i < led_buffer->leds_count; i++) {
		buffer[length++] = (uint8_t) led_buffer->leds[i].key;
		buffer[length++] = led_buffer->leds[i].intensity;
		buffer[length++] = led_buffer->leds[i].intensity;
		buffer[length++] = led_buffer->leds[i].intensity;
	}

	if ((res = hid_write(device->dev_0, buffer,
		gkbd_address_group_message_size(led_buffer->address_group))) < 0)
		return false;
	return true;
}

bool gkbd_write_leds(gkbd_device * device, gkbd_led * leds, size_t length) {
	if (length < 1) return true;
	bool result = true;
	gkbd_led_buffer logo_buffer = gkbd_led_buffer_create(ADDRESS_GROUP_LOGO);
	gkbd_led_buffer indicators_buffer = gkbd_led_buffer_create(ADDRESS_GROUP_INDICATORS);
	gkbd_led_buffer multimedia_buffer = gkbd_led_buffer_create(ADDRESS_GROUP_MULTIMEDIA);
	gkbd_led_buffer gkeys_buffer = gkbd_led_buffer_create(ADDRESS_GROUP_GKEYS);
	gkbd_led_buffer keys_buffer = gkbd_led_buffer_create(ADDRESS_GROUP_KEYS);

	gkbd_led_buffer * buffer;
	gkbd_led remaining_leds[length];
	size_t remaining_count = 0;

	for (int i = 0; i < length; i++) {

		switch (gkbd_get_address_group(leds[i].key)) {
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

	result &= gkbd_write_led_buffer(device, &logo_buffer);
	result &= gkbd_write_led_buffer(device, &indicators_buffer);
	result &= gkbd_write_led_buffer(device, &multimedia_buffer);
	result &= gkbd_write_led_buffer(device, &gkeys_buffer);
	result &= gkbd_write_led_buffer(device, &keys_buffer);
	result &= gkbd_commit(device);

	gkbd_led_buffer_destroy(&logo_buffer);
	gkbd_led_buffer_destroy(&indicators_buffer);
	gkbd_led_buffer_destroy(&multimedia_buffer);
	gkbd_led_buffer_destroy(&gkeys_buffer);
	gkbd_led_buffer_destroy(&keys_buffer);

	gkbd_write_leds(device, remaining_leds, remaining_count);

	return result;
}

bool gkbd_write_led(gkbd_device * gkbd_device, gkbd_led led) {
	return gkbd_write_leds(gkbd_device, &led, 1);
}


