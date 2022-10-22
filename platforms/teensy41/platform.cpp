
#include "platform.h"
#include "usb_serial.h"
#include "HardwareSerial.h"
#include "SdioTeensy.h"
#include "usb/USBHost_t36.h"
#include "drivers/disks.h"
#include "drivers/filesystems/fat32.h"

#include <string.h>

USBHost myusb;
USBHub hub1(myusb);
USBHub hub2(myusb);
KeyboardController keyboard1(myusb);
MouseController mouse1(myusb);
USBHIDParser hid0(myusb);
USBHIDParser hid1(myusb);
//MIDIDevice midi1(myusb);


static SdioCard builtinSdCard;
extern HardwareSerial Serial1;
extern HardwareSerial Serial2;
extern HardwareSerial Serial3;
extern HardwareSerial Serial4;
extern HardwareSerial Serial5;
extern HardwareSerial Serial6;
extern HardwareSerial Serial7;
extern HardwareSerial Serial8;

cid_t m_cid;

bool cidDmp() {
	Serial.printf("\nManufacturer ID: %#X\n", (int)m_cid.mid);
	Serial.printf("OEM ID: %i %i\n", m_cid.oid[0], m_cid.oid[1]);
	Serial.printf("Product: % 5s\n", m_cid.pnm);
	Serial.printf("Version: %i.%i\n", (int)m_cid.prvN(), (int)m_cid.prvM());
	Serial.printf("Serial number: %x\n", m_cid.psn());
	/*Serial.printf("Manufacturing date: ");
	cout << int(m_cid.mdt_month) << '/';
	cout << (2000 + m_cid.mdt_year_low + 10 * m_cid.mdt_year_high) << endl;
	cout << endl;*/
	return true;
}

_get_devices_retval_t _platform_get_devices() {
	_get_devices_retval_t result;

	delay(1000);

	result.device_list = (Device**)malloc(sizeof(Device*) * 11);
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
	result.device_list[10] = &keyboard1;
	result.num_devices = 11;
	result.primary_stream = &Serial;
	result.was_init_ok = true;

	return result;
}

