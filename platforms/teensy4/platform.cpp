
#include "platform.h"
#include "usb_serial.h"


_get_devices_retval_t _platform_get_devices() {
	_get_devices_retval_t result;

	result.device_list = (Device**)malloc(sizeof(Device*) * 1);
	result.device_list[0] = &Serial;
	result.num_devices = 1;
	result.primary_stream = &Serial;
	result.was_init_ok = true;
	return result;
}

