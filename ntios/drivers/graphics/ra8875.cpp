
#include "ra8875_registers.h"
#include "drivers/graphics/ra8875.h"
#include "ntios.h"

RA8875::RA8875(SPIBusDevice* spi_bus, GPIODevice* gpio, int rst, int cs, timing_params_t* timing) {

	width = timing->width;
	height = timing->height;
	voffset = timing->voffset;
	this->spi = spi_bus;
	this->gpio = gpio;
	this->cs = cs;
	this->rst = rst;

	gpio->pinMode(cs, GPIO_PIN_MODE_OUTPUT);
	gpio->writePin(cs, true);
	gpio->pinMode(rst, GPIO_PIN_MODE_OUTPUT);

	hardware_reset();
	
	writeReg(RA8875_PLLC1, timing->PLLC1);
	delay(1);
	writeReg(RA8875_PLLC2, RA8875_PLLC2_DIV4);
	delay(1);
	writeReg(RA8875_SYSR, RA8875_SYSR_16BPP | RA8875_SYSR_MCU8);

	writeReg(RA8875_PCSR, timing->pixclk);
	delay(1);

	/* Horizontal settings registers */
	// H width: (HDWR + 1) * 8 = 480
	writeReg(RA8875_HDWR, (width / 8) - 1);
	writeReg(RA8875_HNDFTR, RA8875_HNDFTR_DE_HIGH + timing->hsync_finetune);
	// H non-display: HNDR * 8 + HNDFTR + 2 = 10
	writeReg(RA8875_HNDR, (timing->hsync_nondisp - timing->hsync_finetune - 2) / 8);
	// Hsync start: (HSTR + 1)*8
	writeReg(RA8875_HSTR, timing->hsync_start / 8 - 1);
	// HSync pulse width = (HPWR+1) * 8
	writeReg(RA8875_HPWR, RA8875_HPWR_LOW + (timing->hsync_pw / 8 - 1));

	/* Vertical settings registers */
	writeReg(RA8875_VDHR0, (uint16_t)(height - 1 + voffset) & 0xFF);
	writeReg(RA8875_VDHR1, (uint16_t)(height - 1 + voffset) >> 8);
	// V non-display period = VNDR + 1
	writeReg(RA8875_VNDR0, timing->vsync_nondisp - 1);
	writeReg(RA8875_VNDR1, timing->vsync_nondisp >> 8);
	// Vsync start position = VSTR + 1
	writeReg(RA8875_VSTR0, timing->vsync_start - 1);
	writeReg(RA8875_VSTR1, timing->vsync_start >> 8);
	// Vsync pulse width = VPWR + 1
	writeReg(RA8875_VPWR, RA8875_VPWR_LOW + timing->vsync_pw - 1);

	/* Set active window X */
	// horizontal start point
	writeReg(RA8875_HSAW0, 0);
	writeReg(RA8875_HSAW1, 0);
	// horizontal end point
	writeReg(RA8875_HEAW0, (uint16_t)(width - 1) & 0xFF);
	writeReg(RA8875_HEAW1, (uint16_t)(width - 1) >> 8);

	/* Set active window Y */
	// vertical start point
	writeReg(RA8875_VSAW0, 0 + voffset);
	writeReg(RA8875_VSAW1, 0 + voffset);
	// vertical end point
	writeReg(RA8875_VEAW0, (uint16_t)(height - 1 + voffset) & 0xFF);
	writeReg(RA8875_VEAW1, (uint16_t)(height - 1 + voffset) >> 8);

	/* Clear the entire window */
	writeReg(RA8875_MCLR, RA8875_MCLR_START | RA8875_MCLR_FULL);

	delay(500);
	displayOn(true);

	// Enable the brightness PWM
	writeReg(RA8875_P1CR, RA8875_P1CR_ENABLE | (RA8875_PWM_CLK_DIV1024 & 0xF));
}

void RA8875::hardware_reset() {
	gpio->writePin(rst, false);
	delay(100);
	gpio->writePin(rst, true);
	delay(100);
}

void RA8875::clearScreen(uint16_t color) {
	fillRect(0, 0, width - 1, height - 1, color);
}

void RA8875::setPixel(int x, int y, uint16_t color) {
	char write_pixel_cmd[3] = { RA8875_DATAWRITE, color >> 8, color };

	writeReg(RA8875_CURH0, x);
	writeReg(RA8875_CURH1, x >> 8);
	writeReg(RA8875_CURV0, y);
	writeReg(RA8875_CURV1, y >> 8);

	writeCommand(RA8875_MRWC);

	writeBytes(write_pixel_cmd, 3);
}

void RA8875::setPixels(int x, int y, uint16_t* colors, int len) {

	writeReg(RA8875_CURH0, x);
	writeReg(RA8875_CURH1, x >> 8);
	writeReg(RA8875_CURV0, y);
	writeReg(RA8875_CURV1, y >> 8);

	writeReg(RA8875_MWCR0, (readReg(RA8875_MWCR0) & ~RA8875_MWCR0_DIRMASK) | RA8875_MWCR0_LRTD);

	writeCommand(RA8875_MRWC);

	spi->lock();
	gpio->writePin(cs, false);
	char tmpSPIVal = RA8875_DATAWRITE;
	spi->exchange(1, &tmpSPIVal, nullptr);
	while (len--) {
		uint16_t color = *colors++;
		char bytes[2] = { color >> 8, color & 255 };
		spi->exchange(2, bytes, nullptr);
	}
	gpio->writePin(cs, true);
	spi->unlock();
}

void RA8875::fillRect(int x1, int y1, int x2, int y2, uint16_t color) {

	int x = min(x1, x2);
	int y = min(y1, y2);
	int w = abs(x2 - x1) + 1;
	int h = abs(y2 - y1) + 1;

	/* Set X */
	writeCommand(0x91);
	writeData(x);
	writeCommand(0x92);
	writeData(x >> 8);

	/* Set Y */
	writeCommand(0x93);
	writeData(y);
	writeCommand(0x94);
	writeData(y >> 8);

	/* Set X1 */
	writeCommand(0x95);
	writeData(w);
	writeCommand(0x96);
	writeData((w) >> 8);

	/* Set Y1 */
	writeCommand(0x97);
	writeData(h);
	writeCommand(0x98);
	writeData((h) >> 8);

	/* Set Color */
	writeCommand(0x63);
	writeData((color & 0xf800) >> 11);
	writeCommand(0x64);
	writeData((color & 0x07e0) >> 5);
	writeCommand(0x65);
	writeData((color & 0x001f));

	/* Draw! */
	writeCommand(RA8875_DCR);
	writeData(0xB0); // Use 0x90 if you just want to draw the edges

	/* Wait for the command to finish */
	waitPoll(RA8875_DCR, RA8875_DCR_LINESQUTRI_STATUS);
}

void RA8875::drawBitmap16(int x, int y, int w, int h, uint16_t* data) {
	(void)x;
	(void)y;
	(void)w;
	(void)h;
	(void)data;
}

void RA8875::writeBytes(char* data, uint8_t len) {
	spi->lock();
	gpio->writePin(cs, false);
	spi->exchange(len, data, nullptr);
	gpio->writePin(cs, true);
	spi->unlock();
}

void RA8875::displayOn(bool on) {
	if (on) {
		writeReg(RA8875_PWRR, RA8875_PWRR_NORMAL | RA8875_PWRR_DISPON);
		writeReg(RA8875_GPIOX, 1);
	} else {
		writeReg(RA8875_PWRR, RA8875_PWRR_NORMAL | RA8875_PWRR_DISPOFF);
		writeReg(RA8875_GPIOX, 0);
	}
}

void RA8875::setBrightness(float percent) {
	writeReg(RA8875_P1DCR, (uint8_t)(percent * 2.55f));
}

/**************************************************************************/
/*!
      Waits for screen to finish by polling the status!
      @param regname The register name to check
      @param waitflag The value to wait for the status register to match
      @return True if the expected status has been reached
*/
/**************************************************************************/
bool RA8875::waitPoll(uint8_t regname, uint8_t waitflag) {
	/* Wait for the command to finish */
	while (1) {
		uint8_t temp = readReg(regname);
		if (!(temp & waitflag))
			return true;
	}

	return false; // MEMEFIX: yeah i know, unreached! - add timeout?
}

/************************* Low Level **************************************/
/* You will notice this is largely copied from Adafruit's RA8875 driver.  */

/**************************************************************************/
/*!
    Write data to the specified register
    @param reg Register to write to
    @param val Value to write
*/
/**************************************************************************/
void RA8875::writeReg(uint8_t reg, uint8_t val) {
	writeCommand(reg);
	writeData(val);
}

/**************************************************************************/
/*!
    Set the register to read from
    @param reg Register to read
    @return The value
*/
/**************************************************************************/
uint8_t RA8875::readReg(uint8_t reg) {
	writeCommand(reg);
	return readData();
}

/**************************************************************************/
/*!
    Write data to the current register
    @param d Data to write
*/
/**************************************************************************/
void RA8875::writeData(uint8_t d) {
	spi->lock();
	gpio->writePin(cs, false);

	char data[2] = { RA8875_DATAWRITE, d };
	spi->exchange(2, data, nullptr);

	gpio->writePin(cs, true);
	spi->unlock();
}

/**************************************************************************/
/*!
    Read the data from the current register
    @return The Value
*/
/**************************************************************************/
uint8_t RA8875::readData(void) {
	spi->lock();
	gpio->writePin(cs, false);

	char in_data[2] = { RA8875_DATAREAD, 0x0 };
	char out_data[2];
	spi->exchange(2, in_data, out_data);

	gpio->writePin(cs, true);
	spi->unlock();

	return out_data[1];
}

/**************************************************************************/
/*!
    Write a command to the current register
    @param d The data to write as a command
 */
/**************************************************************************/
void RA8875::writeCommand(uint8_t d) {
	spi->lock();
	gpio->writePin(cs, false);

	char data[2] = { RA8875_CMDWRITE, d };
	spi->exchange(2, data, nullptr);

	gpio->writePin(cs, true);
	spi->unlock();
}

/**************************************************************************/
/*!
    Read the status from the current register
    @return The value
 */
/**************************************************************************/
uint8_t RA8875::readStatus(void) {
	spi->lock();
	gpio->writePin(cs, false);

	char in_data[2] = { RA8875_CMDREAD, 0x0 };
	char out_data[2];
	spi->exchange(2, in_data, out_data);

	gpio->writePin(cs, true);
	spi->unlock();

	return out_data[1];
}
