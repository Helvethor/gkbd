#pragma once

#include "gkbd.h"
#include "reader.h"

typedef struct gkbd_device {
	hid_device * dev_0;
	hid_device * dev_1;
	gkbd_reader * reader_0;
	gkbd_reader * reader_1;
} gkbd_device;
