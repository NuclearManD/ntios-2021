
// We are not using the exact header name here because we may conflict with some
// Arduino libs by having a GRAPHICS_H macro.
#ifndef NTIOS_GRAPHICS_H
#define NTIOS_GRAPHICS_H

#include "color.h"
#include "font.h"
#include "drivers.h"

class GraphicsDisplayDevice: public StreamDevice {
protected:
	BitmapFont& font = *(new DefaultFont());
	int cursor_x = 0, cursor_y = 0;
	uint16_t text_color = 0xFFFF;

public:
	int read() { return -1; }
	int available() { return 0; }
	int peek() { return -1; }

	int getType() { return DEV_TYPE_GRAPHICS; }

	virtual void clearScreen(uint16_t color = 0) = 0;

	virtual int setTextCursor(uint32_t x, uint32_t y);
	virtual int setTextCursorPixels(uint32_t x, uint32_t y);
	virtual void setFont(BitmapFont& font) { this->font = font; }
	virtual void setTextColor(uint16_t color) { this->text_color = color; }
	virtual size_t write(uint8_t val);

	// By default we can't do this:
	virtual void scrollDownPixels(uint32_t pixels) { (void) pixels; }

	virtual int getTextLines() { return getHeight() / font.getCharHeight(); }
	virtual int getTextColumns() { return getWidth() / font.getCharWidth(); }
	virtual uint32_t getWidth() = 0;
	virtual uint32_t getHeight() = 0;

	virtual void setPixel(int x, int y, uint16_t color) = 0;

	virtual void fillRect(int x1, int y1, int x2, int y2, uint16_t color);
	virtual void drawVerticalLine(int x, int y1, int y2, uint16_t color);
	virtual void drawHorizontalLine(int y, int x1, int x2, uint16_t color);
	virtual void drawRect(int x1, int y1, int x2, int y2, uint16_t color);

	// Draw a bitmap of 16-bit colors
	// Data comes row-by-row, x and y are at the upper left corner of the image
	virtual void drawBitmap16(int x, int y, int w, int h, uint16_t* data) = 0;
};

#endif
