
#ifndef FONT_H
#define FONT_H

#include <stdint.h>

class GraphicsDisplayDevice;

class BitmapFont {
public:
	virtual char getCharWidth() = 0;
	virtual char getCharHeight() = 0;
	virtual void drawChar(GraphicsDisplayDevice* disp, uint16_t color, int x, int y, char c) = 0;
};

class DefaultFont: public BitmapFont {
public:
	char getCharWidth() { return 8; }
	char getCharHeight() { return 12; }
	void drawChar(GraphicsDisplayDevice* disp, uint16_t color, int sx, int sy, char c);
};

#endif
