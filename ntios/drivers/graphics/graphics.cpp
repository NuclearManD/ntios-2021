
#include "graphics.h"

/*void GraphicsDisplayDevice::fillRect(int x1, int y1, int x2, int y2, color_t color) {
	for (int x = x1; x <= x2; x++) {
		for (int y = y1; y <= y2; y++) {
			setPixel(x, y, color);
		}
	}
}

void GraphicsDisplayDevice::fillRect(int x1, int y1, int x2, int y2, uint8_t color) {
	for (int x = x1; x <= x2; x++) {
		for (int y = y1; y <= y2; y++) {
			setPixel(x, y, color);
		}
	}
}*/

void GraphicsDisplayDevice::fillRect(int x1, int y1, int x2, int y2, uint16_t color) {
	for (int x = x1; x <= x2; x++) {
		for (int y = y1; y <= y2; y++) {
			setPixel(x, y, color);
		}
	}
}

void GraphicsDisplayDevice::drawVerticalLine(int x, int y1, int y2, uint16_t color) {
	for (int y = y1; y <= y2; y++) {
		setPixel(x, y, color);
	}
}

void GraphicsDisplayDevice::drawHorizontalLine(int y, int x1, int x2, uint16_t color) {
	for (int x = x1; x <= x2; x++) {
		setPixel(x, y, color);
	}
}

void GraphicsDisplayDevice::drawRect(int x1, int y1, int x2, int y2, uint16_t color) {
	drawVerticalLine(x1, y1, y2, color);
	drawVerticalLine(x2, y1, y2, color);
	drawHorizontalLine(y1, x1, x2, color);
	drawHorizontalLine(y2, x1, x2, color);
}

int GraphicsDisplayDevice::setTextCursor(uint32_t x, uint32_t y) {
	if (x >= getTextColumns() || y >= getTextLines())
		return ERR_OUT_OF_BOUNDS;
	flush();

	cursor_x = x * font.getCharWidth();
	cursor_y = y * font.getCharHeight();

	return 0;
}

int GraphicsDisplayDevice::setTextCursorPixels(uint32_t x, uint32_t y) {
	if (x >= getWidth() || y >= getHeight())
		return ERR_OUT_OF_BOUNDS;
	flush();

	cursor_x = x;
	cursor_y = y;

	return 0;
}

size_t GraphicsDisplayDevice::write(uint8_t val) {
	int cw = font.getCharWidth();

	if (val == '\n') {
		cursor_y += font.getCharHeight();
		cursor_x = 0;
	} else if (val == 0 || val == '\r')
		; // Skip these chars
	else if (val == '\t') {
		fillRect(cursor_x, cursor_y, cursor_x + cw * 4, cursor_y + font.getCharHeight(), 0);
		cursor_x += cw * 4;
	} else if (val == 8) {
		// Backspace
		cursor_x -= cw;
		fillRect(cursor_x, cursor_y, cursor_x + cw, cursor_y + cw, 0);
	} else {
		fillRect(cursor_x, cursor_y, cursor_x + cw, cursor_y + font.getCharHeight(), 0);
		font.drawChar(this, text_color, cursor_x, cursor_y, val);
		cursor_x += cw;
	}

	if (cursor_x + cw > getWidth()) {
		cursor_y++;
		cursor_x = 0;
	}

	if (cursor_y + font.getCharHeight() > getHeight()) {
		scrollDownPixels(cursor_y + font.getCharHeight() - getHeight());
		cursor_y = getHeight() - font.getCharHeight();
	}

	return 1;
}
