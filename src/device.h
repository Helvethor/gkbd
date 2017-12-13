#pragma once

#include "g610.h"
#include "reader.h"

typedef struct g610_device {
	hid_device * dev_0;
	hid_device * dev_1;
	g610_reader * reader_0;
	g610_reader * reader_1;
} g610_device;
