
#ifndef NTIOS_PLATFORM_H
#define NTIOS_PLATFORM_H

#include "drivers.h"


// These things MUST be defined by the platform.


typedef struct {
	Device** device_list;
	int num_devices;
	bool was_init_ok;
	StreamDevice* primary_stream;
} _get_devices_retval_t;

/*
 * _platform_get_devices
 *
 * This function tells main() what devices are present, so NTIOS
 * can load it's hardware device list.
 *
 * Note that this is NOT a C function, and does NOT use C linkage.
 */
_get_devices_retval_t _platform_get_devices();


#endif
