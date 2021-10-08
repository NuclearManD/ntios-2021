
#include "drivers/graphics/canvas.h"
#include "math.h"

DefaultFont* _global_default_font = new DefaultFont();

void Canvas::clearScreen(uint16_t color) {
	gfx_device->fillRect(offset_x, offset_y, offset_x + width, offset_y + height, color);
}

void Canvas::fillRect(uint x1, uint y1, uint x2, uint y2, uint16_t color) {
	if (x1 > width)
		return;
	if (y1 > height)
		return;

	if (x2 > width)
		x2 = width;
	if (y2 > height)
		y2 = height;

	x1 += offset_x;
	x2 += offset_x;
	y1 += offset_y;
	y2 += offset_y;

	gfx_device->fillRect(x1, y1, x2, y2, color);
}

void Canvas::setPixel(uint x, uint y, uint16_t color) {
	if (x < width && y < height)
		gfx_device->setPixel(x + offset_x, y + offset_y, color);
}

void Canvas::drawCircle(uint x, uint y, uint r, uint16_t color) {

	if (x - r > width)
		return;
	if (y - r > height)
		return;
	if (r == 0) {
		setPixel(x, y, color);
		return;
	}

	x += offset_x;
	y += offset_y;
	
	uint x1 = x - r;
	uint x2 = x + r;
	uint y1 = y - r;
	uint y2 = y + r;

	if (x2 > width)
		x2 = width;
	if (y2 > height)
		y2 = height;

	for (uint px = x1; x <= x2; px++) {
		for (uint py = y1; y <= y2; py++) {
			uint dx = px - x;
			uint dy = py - y;
			if (sqrtf(dx * dx + dy * dy) <= r)
				gfx_device->setPixel(x, y, color);
		}
	}
}

void Canvas::drawBitmap16(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint16_t* data) {
	if (x > width)
		return;
	if (y > height)
		return;

	x += offset_x;
	y += offset_y;

	gfx_device->drawBitmap16(x, y, w, h, data);
}

size_t Canvas::write(uint8_t c) {
	if (cursor_x < width && cursor_y < height) {
		font->drawChar(gfx_device, text_color, cursor_x + offset_x, cursor_y + offset_y, c);
		cursor_x += 8;
		return 1;
	}
	return 0;
}
