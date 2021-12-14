//
// Created by nuclaer on 10/8/21.
//

#include "platform.h"

#include <fcntl.h>
#include <unistd.h>


class StandardIOPort: public StreamDevice {
public:
	StandardIOPort() {
		int flags = fcntl(0, F_GETFL, 0);
		fcntl(0, F_SETFL, flags | O_NONBLOCK);
	}

	int read() {
		uint8_t c;
		if (has_char) {
			has_char = false;
			return last_char;
		}

		if (::read(0, &c, 1) < 1)
			return -1;
		return c;
	}

	size_t write(uint8_t b) {
		return ::write(1, &b, 1);
	}

	int available() {
		if (has_char)
			return 1;
		if (::read(0, &last_char, 1) < 1)
			return 0;
		has_char = true;
		return 1;
	}
	int peek() {
		if (available())
			return last_char;
		return -1;
	}

	const char* getName() { return "stdio"; }

private:
	uint8_t last_char;
	bool has_char = false;
};


StandardIOPort io;

/*
 * _platform_get_devices
 *
 * This function tells main() what devices are present, so NTIOS
 * can load it's hardware device list.
 *
 * Note that this is NOT a C function, and does NOT use C linkage.
 */
_get_devices_retval_t _platform_get_devices() {
	_get_devices_retval_t result;

	result.device_list = (Device**)malloc(sizeof(Device*) * 1);
	result.device_list[0] = &io;
	result.num_devices = 1;
	result.primary_stream = &io;
	result.was_init_ok = true;
	return result;
}
