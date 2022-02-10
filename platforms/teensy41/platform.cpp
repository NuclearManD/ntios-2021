
#include "platform.h"
#include "usb_serial.h"
#include "SdioTeensy.h"

#include <string.h>


static SdioCard builtinSdCard;

_get_devices_retval_t _platform_get_devices() {
	_get_devices_retval_t result;

	result.device_list = (Device**)malloc(sizeof(Device*) * 2);
	result.device_list[0] = &Serial;
	result.device_list[1] = &builtinSdCard;
	Serial.printf("%p\n", builtinSdCard);
	Serial.printf("%p\n", builtinSdCard.getName());
	Serial.printf("%i\n", strlen(builtinSdCard.getName()));
	Serial.printf("%s\n", builtinSdCard.getName());
	Serial.printf("%hx\n", builtinSdCard.getType());
	result.num_devices = 2;
	result.primary_stream = &Serial;
	result.was_init_ok = true;
	return result;
}

