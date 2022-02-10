
#include "platform.h"
#include "usb_serial.h"
#include "HardwareSerial.h"
#include "SdioTeensy.h"

#include <string.h>


static SdioCard builtinSdCard;
extern HardwareSerial Serial1;
extern HardwareSerial Serial2;
extern HardwareSerial Serial3;
extern HardwareSerial Serial4;
extern HardwareSerial Serial5;
extern HardwareSerial Serial6;
extern HardwareSerial Serial7;
extern HardwareSerial Serial8;

_get_devices_retval_t _platform_get_devices() {
	_get_devices_retval_t result;

	result.device_list = (Device**)malloc(sizeof(Device*) * 10);
	result.device_list[0] = &Serial;
	result.device_list[1] = &Serial1;
	result.device_list[2] = &Serial2;
	result.device_list[3] = &Serial3;
	result.device_list[4] = &Serial4;
	result.device_list[5] = &Serial5;
	result.device_list[6] = &Serial6;
	result.device_list[7] = &Serial7;
	result.device_list[8] = &Serial8;
	result.device_list[9] = &builtinSdCard;
	for(int i = 0; i < 10; i++) {
		//Serial.printf("%i %p\n", i, result.device_list[i]);
	}
	result.num_devices = 10;
	result.primary_stream = &Serial;
	result.was_init_ok = true;
	return result;
}

