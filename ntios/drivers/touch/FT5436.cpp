
#include "drivers/ctp_drivers.h"
#include "ntios.h"

#define FT5436_ADDRESS 0x38

#define FT5436_REG_TOUCH_COUNT 0x01

const char FT5436_setup_command[] = {0, 0};

FT5436Touch::FT5436Touch(I2CBusDevice* i2c, GPIODevice* gpio, int rst, int xconf, int yconf) {
	this->gpio = gpio;
	this->rst = rst;
	this->i2c = i2c;

	if (gpio != nullptr) {
		gpio->pinMode(rst, GPIO_PIN_MODE_OUTPUT);
		gpio->writePin(rst, false);
		delay(25);
		gpio->writePin(rst, true);
		delay(25);
	}

	i2c->write(FT5436_ADDRESS, 2, FT5436_setup_command);

	this->xconf = xconf;
	this->yconf = yconf;
}

void FT5436Touch::update() {
	uint8_t buffer[2 + maxPresses() * 6];
	i2c->read(FT5436_ADDRESS, 2 + maxPresses() * 6, (char*)buffer);
	nPresses = buffer[FT5436_REG_TOUCH_COUNT] & 0x0F;

	for (int i = 0; i < maxPresses(); i++) {
		uint16_t x = ((0x0f & buffer[i*6 + 2]) << 8) | buffer[i*6 + 3];
		uint16_t y = ((0x0f & buffer[i*6 + 4]) << 8) | buffer[i*6 + 5];
		if (xconf)
			x = xconf - x;
		if (yconf)
			y = yconf - y;
		xPresses[i] = x;
		yPresses[i] = y;
		zPresses[i] = ((0x0f & buffer[i*6 + 6]) << 8) | buffer[i*6 + 7];
	}

	i2c->write(FT5436_ADDRESS, 2, FT5436_setup_command);
}
