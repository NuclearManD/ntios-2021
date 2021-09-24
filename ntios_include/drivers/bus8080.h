
#ifndef BUS8080_H
#define BUS8080_H

#include <stdint.h>

// An instance of this should exist for each device on all 8080 style busses,
// assuming the intent is to interface with NTIOS drivers.

class Bus8080 {
public:
	virtual void write(uint16_t address, const uint8_t* data, int size) = 0;
	virtual void write16BigEndian(uint16_t address, const uint16_t* data, int size) = 0;
	virtual void read(uint16_t address, uint8_t* data, int size) = 0;
	virtual void select() = 0;
	virtual void deselect() = 0;
};

#endif
