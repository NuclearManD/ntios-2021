/* Teensyduino Core Library
 * http://www.pjrc.com/teensy/
 * Copyright (c) 2017 PJRC.COM, LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * 1. The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * 2. If the Software is incorporated into a build system that allows
 * selection among a list of target devices, then similar target
 * devices manufactured by PJRC.COM must be included in the list of
 * target devices and selectable in the same manner.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <Arduino.h>
#include <stdio.h>
#include "drivers.h"
#include "ntios.h"
#include "libneon.h"
#include "platform.h"


extern "C" void digitalWrite(int pin, bool val);

extern "C" int main(void)
{
	_get_devices_retval_t devinfo = _platform_get_devices();

	//StreamDevice& ser = *(devinfo.primary_stream);

	delay(500);
	ntios_init(devinfo.device_list, devinfo.num_devices, devinfo.primary_stream);
	//create_new_shell(ser);
	ntios_shell(devinfo.primary_stream);
}

bool launch(int argc, char** argv, StreamDevice* io) {
	(void)argc;
	(void)argv;
	(void)io;
	return false;
}

bool start_function(void (*f)(void* param), void* param, int stack_size) {
	(void)f;
	(void)param;
	(void)stack_size;
	return false;
}

BootloaderMutex* bootloader_make_mutex() {
	return nullptr;
}
