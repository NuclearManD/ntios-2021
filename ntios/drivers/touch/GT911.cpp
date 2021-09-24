
// Based on https://github.com/nik-sharky/arduino-goodix/blob/master/Goodix.cpp
// BECAUSE THE DATASHEET DOESN'T DESCRIBE SHITTTTTTT
// Like, how hard is it to write up a register list and initialization process?
// Geez...
// Thank goodness someone was kind enough to make that Arduino library and put it on GitHub.

#include "../ctp_drivers.h"
#include "../../ntios.h"

#define GT911_ADDRESS 0x5D


#define GOODIX_MAX_HEIGHT   4096
#define GOODIX_MAX_WIDTH    4096
#define GOODIX_INT_TRIGGER    1
#define GOODIX_CONTACT_SIZE   8
#define GOODIX_MAX_CONTACTS   10

#define GOODIX_CONFIG_MAX_LENGTH  240
#define GOODIX_CONFIG_911_LENGTH  186
#define GOODIX_CONFIG_967_LENGTH  228

/* Register defines */
#define GT_REG_CMD  0x8040

#define GT_REG_CFG  0x8047
#define GT_REG_DATA 0x8140


// Write only registers
#define GOODIX_REG_COMMAND        0x8040
#define GOODIX_REG_LED_CONTROL    0x8041
#define GOODIX_REG_PROXIMITY_EN   0x8042

// Read/write registers
// The version number of the configuration file
#define GOODIX_REG_CONFIG_DATA  0x8047
// X output maximum value (LSB 2 bytes)
#define GOODIX_REG_MAX_X        0x8048
// Y output maximum value (LSB 2 bytes)
#define GOODIX_REG_MAX_Y        0x804A
// Maximum number of output contacts: 1~5 (4 bit value 3:0, 7:4 is reserved)
#define GOODIX_REG_MAX_TOUCH    0x804C

// Module switch 1
// 7:6 Reserved, 5:4 Stretch rank, 3 X2Y, 2 SITO (Single sided ITO touch screen), 1:0 INT Trigger mode */
#define GOODIX_REG_MOD_SW1      0x804D
// Module switch 2
// 7:1 Reserved, 0 Touch key */
#define GOODIX_REG_MOD_SW2      0x804E

// Number of debuffs fingers press/release
#define GOODIX_REG_SHAKE_CNT    0x804F

// ReadOnly registers (device and coordinates info)
// Product ID (LSB 4 bytes, GT9110: 0x06 0x00 0x00 0x09)
#define GOODIX_REG_ID           0x8140
// Firmware version (LSB 2 bytes)
#define GOODIX_REG_FW_VER       0x8144

// Current output X resolution (LSB 2 bytes)
#define GOODIX_READ_X_RES       0x8146
// Current output Y resolution (LSB 2 bytes)
#define GOODIX_READ_Y_RES       0x8148
// Module vendor ID
#define GOODIX_READ_VENDOR_ID   0x814A

#define GOODIX_READ_COORD_ADDR  0x814E

/* Commands for REG_COMMAND */
//0: read coordinate state
#define GOODIX_CMD_READ         0x00
// 1: difference value original value
#define GOODIX_CMD_DIFFVAL      0x01
// 2: software reset
#define GOODIX_CMD_SOFTRESET    0x02
// 3: Baseline update
#define GOODIX_CMD_BASEUPDATE   0x03
// 4: Benchmark calibration
#define GOODIX_CMD_CALIBRATE    0x04
// 5: Off screen (send other invalid)
#define GOODIX_CMD_SCREEN_OFF   0x05

/* When data needs to be sent, the host sends command 0x21 to GT9x,
 * enabling GT911 to enter "Approach mode" and work as a transmitting terminal */
#define GOODIX_CMD_HOTKNOT_TX   0x21

struct GTPoint {
  // 0x814F-0x8156, ... 0x8176 (5 points) 
  uint8_t trackId;
  uint8_t x_low;
  uint8_t x_high;
  uint8_t y_low;
  uint8_t y_high;
  uint8_t area_low;
  uint8_t area_high;
  uint8_t reserved;
};

#include <Arduino.h>

GT911Touch::GT911Touch(I2CBusDevice* i2c, GPIODevice* gpio, int rst, int intpin, int xconf, int yconf) {
	this->gpio = gpio;
	this->rst = rst;
	this->i2c = i2c;

	if (gpio != nullptr) {
		gpio->pinMode(rst, GPIO_PIN_MODE_OUTPUT);
		gpio->pinMode(intpin, GPIO_PIN_MODE_OUTPUT);
		
		// Reset
		gpio->writePin(rst, false);
		gpio->writePin(intpin, false);
		delay(11);

		// set the I2C address to 0xBA/0xBB
		gpio->writePin(intpin, false);
		delay(1);

		// End reset condition
		gpio->writePin(rst, true);
		delay(6);
		
		// Pull int pin low for more than 50ms, as specified in datasheet
		delay(51);
		gpio->pinMode(intpin, GPIO_PIN_MODE_INPUT);

		//Serial.println("GT911 reset complete");
	}

	activation_timer = millis() + 200;

	this->xconf = xconf;
	this->yconf = yconf;
}

void GT911Touch::update() {
	// Wait for the chip to initialize
	if (activation_timer > millis())
		return;
	else
		// This saves us from certain edge cases
		activation_timer = 0;

	uint8_t regState;

	if (!read(GOODIX_READ_COORD_ADDR, &regState, 1)) {
		//Serial.println("Read error on I2C bus");
		return;
	}

	//Serial.printf("v = 0x%02hhx\n", regState);

	if (!(regState & 0x80))
		return;

	uint8_t n_touches = regState & 0x0f;

	if (n_touches > 0) {
		GTPoint data[n_touches];

		if (!read(GOODIX_READ_COORD_ADDR + 1, (uint8_t*)data, sizeof(GTPoint) * n_touches))
			return;

		for (int i = 0; i < n_touches; i++) {
			uint16_t x = (data[i].x_high << 8) | data[i].x_low;
			uint16_t y = (data[i].y_high << 8) | data[i].y_low;
			if (xconf)
				x = xconf - x;
			if (yconf)
				y = yconf - y;
			xPresses[i] = x;
			yPresses[i] = y;
			zPresses[i] = (data[i].area_high << 8) | data[i].area_low;
		}
	}

	write(GOODIX_READ_COORD_ADDR, 0);

	nPresses = n_touches;
}

bool GT911Touch::read(uint16_t address, uint8_t* buffer, int length) {
	uint8_t adrbuf[2] = {address >> 8, address & 255};

	i2c->lock();

	if (!i2c->write(GT911_ADDRESS, 2, adrbuf)) {
		//Serial.println("Write failure.");
		i2c->unlock();
		return false;
	}

	int pos = 0;
	int errors_left = 3;
	//Serial.printf("Reading %i bytes\n", length);
	while (pos < length) {
		int more = i2c->readSome(GT911_ADDRESS, length - pos, &(buffer[pos]));
		if (more == 0)
			if (errors_left-- == 0) {
				//Serial.printf("Read failure, %i bytes read of %i\n", pos, length);
				i2c->unlock();
				return false;
			}
		pos += more;
	}
	//Serial.printf("Read %i bytes successfully\n", length);

	i2c->unlock();
	return true;
}

bool GT911Touch::write(uint16_t address, uint8_t* buffer, int length) {
	uint8_t fullbuf[length + 2];

	fullbuf[0] = address >> 8;
	fullbuf[1] = address & 255;
	memcpy(&(fullbuf[2]), buffer, length);

	i2c->lock();

	if (!i2c->write(GT911_ADDRESS, length + 2, fullbuf)) {
		i2c->unlock();
		return false;
	}

	i2c->unlock();

	return true;
}

bool GT911Touch::write(uint16_t address, uint8_t val) {
	uint8_t fullbuf[3];

	fullbuf[0] = address >> 8;
	fullbuf[1] = address & 255;
	fullbuf[2] = val;

	i2c->lock();

	if (!i2c->write(GT911_ADDRESS, 3, fullbuf)) {
		i2c->unlock();
		return false;
	}

	i2c->unlock();

	return true;
}
