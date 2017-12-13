#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "hidapi/hidapi.h"
#include "g610.h"

#define WSTR_MAX_LENGTH 255
#define HID_BUFFER_LENGTH 65

int main(int argc, char* argv[]) {
	int res;
	unsigned char buf[HID_BUFFER_LENGTH] = { 0 }, buf2[HID_BUFFER_LENGTH] = { 0 };
	wchar_t wstr[WSTR_MAX_LENGTH];
	g610_device * g610;
	int i;

	res = hid_init();
	if (res < 0) {
		printf("Could not init hidapi: %d\n", res);
		return -1;
	}

	hid_device * hid = hid_open_path("/dev/hidraw4");
	if (!hid)
		return -1;
	uint8_t buff[G610_READ_MAX_SIZE] = { 0xff };
	if (hid_write(hid, buff, G610_READ_MAX_SIZE) < 0)
		return -4;


	struct hid_device_info *devs, *dev;

	g610 = g610_open();
	if (!g610) {
		fprintf(stderr, "Couldn't open g610\n");
		return -1;
	}

	g610_led leds[] = {
		{
			.key = KEY_Q,
			.intensity = 0xff
		},
		{
			.key = KEY_W,
			.intensity = 0xbf
		},
		{
			.key = KEY_E,
			.intensity = 0x7f
		},
		{
			.key = KEY_R,
			.intensity = 0x5f
		},
		{
			.key = KEY_T,
			.intensity = 0x2f
		},
		{
			.key = KEY_Y,
			.intensity = 0x0f
		}
	};
	if (!g610_set_leds(g610, leds, sizeof(leds) / sizeof(g610_led))) {
		fprintf(stderr, "Couldn't set leds\n");
	}

	g610_key keys[G610_READ_MAX_SIZE];
	g610_led mleds[G610_READ_MAX_SIZE];

	while (true) {
		res = g610_get_keys(g610, keys, sizeof(keys) / sizeof(g610_key));
		printf("read %2d: ", res);
		for (int i = 0; i < res; i++) {
			printf("%04X ", (uint16_t) keys[i]);
			mleds[i].key = keys[i];
			mleds[i].intensity = 0xff;
		}
		g610_set_leds(g610, mleds, res);
		printf("\n");
		sleep(1);
	}

	res = hid_exit();
	return res;
}
