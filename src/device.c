#include "device.h"
#include "reader.h"

gkbd_device * gkbd_open() {

	gkbd_device * device = calloc(1, sizeof(gkbd_device));
	if (!device)
		return NULL;

	struct hid_device_info * devs = hid_enumerate(G610_VENDOR_ID, G610_PRODUCT_ID);
	struct hid_device_info * dev = devs;

	while (dev) {
		if (!device->dev_0 && dev->interface_number == 0) {
			device->dev_0 = hid_open_path(dev->path);
			if (!device->dev_0)
				break;
		}
		if (!device->dev_1 && dev->interface_number == 1) {
			device->dev_1 = hid_open_path(dev->path);
			if (!device->dev_1)
				break;
		}
		dev = dev->next;
	}

	hid_free_enumeration(devs);

	if (!device->dev_0 || !device->dev_1) {
		gkbd_close(device);
		device = NULL;
	}

	return device;
}

void gkbd_close(gkbd_device * device) {

	if (device->reader_0)
		gkbd_reader_destroy(device->reader_0);
	if (device->reader_1)
		gkbd_reader_destroy(device->reader_1);

	hid_close(device->dev_0);
	hid_close(device->dev_1);

	free(device);
}


