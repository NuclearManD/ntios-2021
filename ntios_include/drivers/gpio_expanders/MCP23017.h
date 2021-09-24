
#ifndef MCP23017_H
#define MCP23017_H

#include <stdint.h>
#include "../../drivers.h"

class MCP23017: public GPIODevice {
public:
	MCP23017(I2CBusDevice* i2cdev, uint8_t addr = 0x20);

	// Returns the number of pins on the device
	int pinCount() { return 16; };
	const char* getName() { return name; }

	// Sets the mode of the pin specified.  Returns true on success.
	// On failure: mode is unchanged and function returns false.
	bool pinMode(int pin, int mode);

	// Returns true if the pin is HIGH and false if the pin is LOW.
	bool readPin(int pin);

	// Attempt to write to a pin,  Returns true on success, false on error.
	bool writePin(int pin, bool state);

private:
	I2CBusDevice* i2c;
	uint8_t address;
	char name[32];
};

#endif
