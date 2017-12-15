#pragma once

#include "gkbd.h"

typedef struct gkbd_led_buffer {
	gkbd_address_group address_group;
	gkbd_led * leds;
	size_t leds_length;
	size_t leds_count;
} gkbd_led_buffer;


