
#ifndef CANVAS_H
#define CANVAS_H

#include "graphics.h"
#include "color.h"
#include "font.h"

extern DefaultFont* _global_default_font;

typedef unsigned int uint;

class Canvas: public StreamDevice {
public:
	BitmapFont* font = _global_default_font;
	uint cursor_x = 1;
	uint cursor_y = 1;
	uint16_t text_color = 0xFFFF;

	Canvas(GraphicsDisplayDevice* gfx, uint x, uint y, uint width, uint height) :
	gfx_device(gfx), offset_x(x), offset_y(y), width(width), height(height) {
		
	}

	void clearScreen(uint16_t color = 0);
	void fillRect(uint x1, uint y1, uint x2, uint y2, uint16_t color);
	void setPixel(uint x, uint y, uint16_t color);
	void drawCircle(uint x, uint y, uint r, uint16_t color);
	void drawBitmap16(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint16_t* data);
	size_t write(uint8_t c);

	int available() { return 0; }
	int peek() { return 0; }
	int read() { return 0; }
	const char* getName() { return "Canvas"; };

	inline void reverse_convert(int &x, int &y) {
		x -= offset_x;
		y -= offset_y;
	}

	inline int getHeight() { return height; }
	inline int getWidth() { return width; }
private:
	GraphicsDisplayDevice* gfx_device;
	uint offset_x;
	uint offset_y;
	uint width, height;
};

#endif
