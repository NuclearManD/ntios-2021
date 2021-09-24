
#include "MCP23017.h"

// registers
#define MCP23017_IODIRA 0x00   // I/O direction register A
#define MCP23017_IPOLA 0x02    // Input polarity port register A
#define MCP23017_GPINTENA 0x04 // Interrupt-on-change pins A
#define MCP23017_DEFVALA 0x06  // Default value register A
#define MCP23017_INTCONA 0x08  // Interrupt-on-change control register A
#define MCP23017_IOCONA 0x0A   // I/O expander configuration register A
#define MCP23017_GPPUA 0x0C    // GPIO pull-up resistor register A
#define MCP23017_INTFA 0x0E    // Interrupt flag register A
#define MCP23017_INTCAPA 0x10  // Interrupt captured value for port register A
#define MCP23017_GPIOA 0x12    // General purpose I/O port register A
#define MCP23017_OLATA 0x14    // Output latch register 0 A

#define MCP23017_IODIRB 0x01   // I/O direction register B
#define MCP23017_IPOLB 0x03    // Input polarity port register B
#define MCP23017_GPINTENB 0x05 // Interrupt-on-change pins B
#define MCP23017_DEFVALB 0x07  // Default value register B
#define MCP23017_INTCONB 0x09  // Interrupt-on-change control register B
#define MCP23017_IOCONB 0x0B   // I/O expander configuration register B
#define MCP23017_GPPUB 0x0D    // GPIO pull-up resistor register B
#define MCP23017_INTFB 0x0F    // Interrupt flag register B
#define MCP23017_INTCAPB 0x11  // Interrupt captured value for port register B
#define MCP23017_GPIOB 0x13    // General purpose I/O port register B
#define MCP23017_OLATB 0x15    // Output latch register 0 B

#define MCP23017_INT_ERR 255 // Interrupt error

static inline uint8_t readReg(I2CBusDevice* i2c, uint8_t devadr, uint8_t addr) {
	char data = addr;
	i2c->write(devadr, 1, &data);
	i2c->read(devadr, 1, &data);
	return data;
}

static inline void writeReg(I2CBusDevice* i2c, uint8_t devadr, uint8_t reg, uint8_t val) {
	char data[] = {reg, val};
	i2c->write(devadr, 2, data);
}

static inline void writeRegMask(I2CBusDevice* i2c, uint8_t devadr, uint8_t reg, uint8_t mask, uint8_t val) {
	uint8_t content = readReg(i2c, devadr, reg);
	content = (content & ~mask) | val;
	writeReg(i2c, devadr, reg, content);
}

static inline uint8_t readRegMask(I2CBusDevice* i2c, uint8_t devadr, uint8_t reg, uint8_t mask) {
	return readReg(i2c, devadr, reg) & mask;
}

static inline void writePinBit(I2CBusDevice* i2c, uint8_t devadr, uint8_t reg, uint8_t pin, uint8_t val) {
	reg |= (pin >> 3) & 1;
	pin &= 7;
	writeRegMask(i2c, devadr, reg, 1 << pin, val << pin);
}

static inline uint8_t readPinBit(I2CBusDevice* i2c, uint8_t devadr, uint8_t reg, uint8_t pin) {
	reg |= (pin >> 3) & 1;
	pin &= 7;
	return readRegMask(i2c, devadr, reg, 1 << pin) >> pin;
}

MCP23017::MCP23017(I2CBusDevice* i2cdev, uint8_t addr) : i2c(i2cdev), address(addr) {

	strcpy(name, "MCP23017 I2C GPIO Expander 0xnn");
	int loc = strlen(name);
	name[loc - 2] = "0123456789ABCDEF"[(address >> 4)];
	name[loc - 1] = "0123456789ABCDEF"[(address & 15)];
	// Set all pins to inputs
	i2c->lock();
	writeReg(i2c, address, MCP23017_IODIRA, 0xff);
	writeReg(i2c, address, MCP23017_IODIRB, 0xff);
	i2c->unlock();
}

// Sets the mode of the pin specified.  Returns true on success.
// On failure: mode is unchanged and function returns false.
bool MCP23017::pinMode(int pin, int mode) {
	char mode_bit;
	char pu_bit = 0;
	switch(mode) {
		case GPIO_PIN_MODE_INPUT_PULLUP:
			pu_bit = 1;
		case GPIO_PIN_MODE_INPUT:
			mode_bit = 1;
			break;
		case GPIO_PIN_MODE_OUTPUT:
			mode_bit = 0;
			break;
		case GPIO_PIN_MODE_INPUT_PULLDOWN:
		case GPIO_PIN_MODE_HIGH_Z:
		default:
			return false;
	};
	i2c->lock();
	writePinBit(i2c, address, MCP23017_IODIRA, pin, mode_bit);
	writePinBit(i2c, address, MCP23017_GPPUA, pin, pu_bit);
	i2c->unlock();
	return true;
}

// Returns true if the pin is HIGH and false if the pin is LOW.
bool MCP23017::readPin(int pin) {
	i2c->lock();
	bool res = readPinBit(i2c, address, MCP23017_GPIOA, pin);
	i2c->unlock();
	return res;
}

// Attempt to write to a pin,  Returns true on success, false on error.
bool MCP23017::writePin(int pin, bool state) {
	if (pin > 15)
		return false;

	uint8_t regbit = (pin >> 3) & 1;
	pin &= 7;
	uint8_t mask = 1 << pin;
	i2c->lock();
	uint8_t content = readReg(i2c, address, MCP23017_OLATA | regbit);
	content = (content & ~mask) | (state << pin);
	writeReg(i2c, address, MCP23017_GPIOA | regbit, content);
	i2c->unlock();
	return true;
}
