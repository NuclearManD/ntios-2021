
#include <stdint.h>

#include "drivers.h"

const char* devTypeToStr(int type) {
	switch (type) {
		case DEV_TYPE_STREAM:
			return "Stream";
		case DEV_TYPE_KEYBOARD:
			return "Keyboard";
		case DEV_TYPE_GRAPHICS:
			return "Graphical Display";
		case DEV_TYPE_SERIAL:
			return "Serial Port";
		case DEV_TYPE_JOINED_STREAM:
			return "Joined Stream";
		case DEV_TYPE_FILESYSTEM:
			return "Filesystem";
		case DEV_TYPE_GPIO:
			return "GPIO Port";
		case DEV_TYPE_PWM_PIN:
			return "PWM";
		case DEV_TYPE_BUS_I2C:
			return "I2C Bus";
		case DEV_TYPE_BUS_SPI:
			return "SPI Bus";
		case DEV_TYPE_BUS_CAN:
			return "CAN Bus";
		case DEV_TYPE_NETWORK:
			return "Network Device";
		case DEV_TYPE_WIFI:
			return "WiFi Modem";
		case DEV_TYPE_USB_HUB:
			return "USB Hub";
		case DEV_TYPE_NAVIGATION:
			return "Navigation System";
		case DEV_TYPE_GPS_RAW:
			return "Raw GPS";
		case DEV_TYPE_IMU:
			return "IMU";
		case DEV_TYPE_AUDIO_OUT:
			return "Audio Output";
		case DEV_TYPE_AUDIO_IN:
			return "Audio Input";
		case DEV_TYPE_ACTUATOR:
			return "Actuator";
		case DEV_TYPE_CONTROL_SYS_SYMMETRIC:
			return "Symmetric Control System";
		case DEV_TYPE_CAPACITIVE_TOUCH:
			return "Capacitive Touch Panel";
		case DEV_TYPE_TOUCH_PANEL:
			return "Touch Panel";
		case DEV_TYPE_SD_CARD:
			return "SD Card";
		default:
			return "Unknown Device";
	}
}

// Device
int Device::sendCommand(int command, char* buffer) {
	(void) command;
	(void) buffer;
	return -1;
}

// StreamDevice

int StreamDevice::getType() {
	return DEV_TYPE_STREAM;
}

// KeyboardDevice

size_t KeyboardDevice::write(uint8_t val) {
	(void)val;
	return 0;
}

void KeyboardDevice::flush() {}

int KeyboardDevice::getType() {
	return DEV_TYPE_KEYBOARD;
}

// JoinedStreamDevice


JoinedStreamDevice::JoinedStreamDevice(StreamDevice* in, StreamDevice* out) {
	input = in;
	output = out;
	name = (char*)malloc(strlen(in->getName()) + strlen(out->getName()) + 2);
	strcpy(name, in->getName());
	strcat(name, "-");
	strcat(name, out->getName());
}

size_t JoinedStreamDevice::write(uint8_t val) {
	return output->write(val);
}

void JoinedStreamDevice::flush() {
	output->flush();
}

void JoinedStreamDevice::update() {
}

int JoinedStreamDevice::getType() {
	return DEV_TYPE_JOINED_STREAM;
}

const char* JoinedStreamDevice::getName() {
	return name;
}

int JoinedStreamDevice::read() {
	return input->read();
}

int JoinedStreamDevice::available() {
	return input->available();
}

int JoinedStreamDevice::peek() {
	return input->peek();
}

StreamDevice* JoinedStreamDevice::getOutput() { return output; }
StreamDevice* JoinedStreamDevice::getInput() { return input; }

// FilesystemDevice

int FileSystemDevice::getType() {
	return DEV_TYPE_FILESYSTEM;
}
