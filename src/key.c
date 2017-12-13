#include "key.h"

g610_address_group g610_get_key_group(g610_key key) {
	return key >> 8;
}

size_t g610_get_group_address(g610_address_group key_group, uint8_t * buffer, size_t length) {
	if (key_group == ADDRESS_GROUP_GKEYS)
		return 0;
	if (length < 8)
		return -1;
	memcpy(buffer, G610_GROUP_ADDRESS[key_group], 8);
	return 8;
}


