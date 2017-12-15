#pragma once

#include "g610.h"

typedef struct g610_led_buffer {
	g610_address_group address_group;
	g610_led * leds;
	size_t leds_length;
	size_t leds_count;
} g610_led_buffer;


