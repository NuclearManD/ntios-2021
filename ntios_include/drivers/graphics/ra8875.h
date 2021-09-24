
#ifndef RA8875_H
#define RA8875_H

#include "graphics.h"
#include "../../drivers.h"
#include "../../ntios.h"

#include "ra8875_registers.h"

// Somewhat based on https://github.com/adafruit/Adafruit_RA8875/blob/master/Adafruit_RA8875.cpp
// which is the Adafruit Arduino Library for their RA8875 breakout

typedef struct {
	uint8_t pixclk;
	uint8_t hsync_start;
	uint8_t hsync_pw;
	uint8_t hsync_finetune;
	uint8_t hsync_nondisp;
	uint8_t vsync_pw;
	uint16_t vsync_nondisp;
	uint16_t vsync_start;
	uint16_t voffset;
	uint16_t width;
	uint16_t height;
	char PLLC1;
} timing_params_t;

class RA8875: public GraphicsDisplayDevice{
public:

	/*
	 * Allocate the timing parameter on the stack, it is only used once.
	 */
	RA8875(SPIBusDevice* spi_bus, GPIODevice* gpio, int rst, int cs, timing_params_t* timing);

	/* 
	 * Factory methods
	 * 
	 * These are more stable and user-friendly than the constructor.
	 * They will not create a driver instance if your parameters are invalid,
	 * and they handle timing configuration for you.
	 */
	static inline RA8875* create480x80(SPIBusDevice* spi_bus, GPIODevice* gpio, int rst, int cs) {
		timing_params_t timing;

		timing.pixclk = RA8875_PCSR_PDATL | RA8875_PCSR_4CLK;
		timing.hsync_nondisp = 10;
		timing.hsync_start = 8;
		timing.hsync_pw = 48;
		timing.hsync_finetune = 0;
		timing.vsync_nondisp = 3;
		timing.vsync_start = 8;
		timing.vsync_pw = 10;
		timing.voffset = 192; // This uses the bottom 80 pixels of a 272 pixel controller
		timing.width = 480;
		timing.height = 80;
		timing.PLLC1 = RA8875_PLLC1_PLLDIV1 + 10;

		if (spi_bus == nullptr || gpio == nullptr || cs == rst) {
			// This detects invalid configuration,
			// or lack of definition for needed drivers
			return nullptr;
		}

		return new RA8875(spi_bus, gpio, rst, cs, &timing);
	}

	static inline RA8875* create480x272(SPIBusDevice* spi_bus, GPIODevice* gpio, int rst, int cs) {
		timing_params_t timing;

		timing.pixclk = RA8875_PCSR_PDATL | RA8875_PCSR_4CLK;
		timing.hsync_nondisp = 10;
		timing.hsync_start = 8;
		timing.hsync_pw = 48;
		timing.hsync_finetune = 0;
		timing.vsync_nondisp = 3;
		timing.vsync_start = 8;
		timing.vsync_pw = 10;
		timing.voffset = 0;
		timing.width = 480;
		timing.height = 272;
		timing.PLLC1 = RA8875_PLLC1_PLLDIV1 + 10;

		if (spi_bus == nullptr || gpio == nullptr || cs == rst) {
			// This detects invalid configuration,
			// or lack of definition for needed drivers
			return nullptr;
		}

		return new RA8875(spi_bus, gpio, rst, cs, &timing);
	}

	static inline RA8875* create800x480(SPIBusDevice* spi_bus, GPIODevice* gpio, int rst, int cs) {
		timing_params_t timing;

		timing.pixclk = RA8875_PCSR_PDATL | RA8875_PCSR_2CLK;
		timing.hsync_nondisp = 26;
		timing.hsync_start = 32;
		timing.hsync_pw = 96;
		timing.hsync_finetune = 0;
		timing.vsync_nondisp = 32;
		timing.vsync_start =23;
		timing.vsync_pw = 2;
		timing.voffset = 0;
		timing.width = 800;
		timing.height = 480;
		timing.PLLC1 = RA8875_PLLC1_PLLDIV1 + 11;

		if (spi_bus == nullptr || gpio == nullptr || cs == rst) {
			// This detects invalid configuration,
			// or lack of definition for needed drivers
			return nullptr;
		}

		return new RA8875(spi_bus, gpio, rst, cs, &timing);
	}

	// Print implementation
	using Print::write; // pull in write(str) and write(buf, size) from Print

	const char* getName() { return "RA8875 Display"; }

	uint32_t getWidth() { return width; }
	uint32_t getHeight() { return height; }

	void clearScreen(uint16_t color);
	void setPixel(int x, int y, uint16_t color);
	void setPixels(int x, int y, uint16_t* colors, int len);
	void fillRect(int x1, int y1, int x2, int y2, uint16_t color);

	void drawBitmap16(int x, int y, int w, int h, uint16_t* data);

	void displayOn(bool on);
	void setBrightness(float percent);

private:
	void writeReg(uint8_t reg, uint8_t val);
	uint8_t readReg(uint8_t reg);
	void writeData(uint8_t d);
	uint8_t readData(void);
	void writeCommand(uint8_t d);
	uint8_t readStatus(void);
	bool waitPoll(uint8_t r, uint8_t f);
	void hardware_reset();

	void drawPixels(int x, int y, int w, uint16_t* data);
	void writeBytes(char* data, uint8_t len);

	uint16_t width, height;
	uint16_t voffset;

	SPIBusDevice* spi;
	GPIODevice* gpio;
	int rst;
	int cs;
};

#endif
