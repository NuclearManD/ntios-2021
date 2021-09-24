
#ifndef CTP_DRIVERS_H
#define CTP_DRIVERS_H

#include "../drivers.h"

class CapacitiveTouchDevice: public Device {
public:
	virtual const char* getName() = 0;
	virtual int getType() { return DEV_TYPE_CAPACITIVE_TOUCH; }
	virtual void update() {};

	virtual int numPresses();
	virtual int maxPresses();
	virtual bool hasZ();
	virtual int pressXCoord(int i);
	virtual int pressYCoord(int i);
	virtual int pressZCoord(int i);
};

class FT5436Touch: public CapacitiveTouchDevice {
public:
	FT5436Touch(I2CBusDevice* i2c, GPIODevice* gpio, int rst, int xconf = 0, int yconf = 0);

	virtual const char* getName() { return "FT5436 Touch Panel"; }
	virtual int maxPresses() { return 5; }
	virtual bool hasZ() { return true; }

	virtual int numPresses() { return nPresses; }
	virtual int pressXCoord(int i) { return xPresses[i]; }
	virtual int pressYCoord(int i) { return yPresses[i]; }
	virtual int pressZCoord(int i) { return zPresses[i]; }

	virtual void update();

protected:
	I2CBusDevice* i2c;
	GPIODevice* gpio;
	int rst;
	int yconf, xconf;

	int nPresses = 0;
	uint16_t xPresses[5];
	uint16_t yPresses[5];
	uint16_t zPresses[5];

};

class GT911Touch: public CapacitiveTouchDevice {
public:
	GT911Touch(I2CBusDevice* i2c, GPIODevice* gpio, int rst, int intpin, int xconf = 0, int yconf = 0);

	virtual const char* getName() { return "GT911 Touch Panel"; }
	virtual int maxPresses() { return 5; }
	virtual bool hasZ() { return true; }

	virtual int numPresses() { return nPresses; }
	virtual int pressXCoord(int i) { return xPresses[i]; }
	virtual int pressYCoord(int i) { return yPresses[i]; }
	virtual int pressZCoord(int i) { return zPresses[i]; }

	virtual void update();

protected:

	bool read(uint16_t address, uint8_t* buffer, int length);
	bool write(uint16_t address, uint8_t* buffer, int length);
	bool write(uint16_t address, uint8_t val);

	I2CBusDevice* i2c;
	GPIODevice* gpio;
	int rst, intpin;
	int yconf, xconf;

	int nPresses = 0;
	uint16_t xPresses[5];
	uint16_t yPresses[5];
	uint16_t zPresses[5];

	uint64_t activation_timer;
};



#endif
