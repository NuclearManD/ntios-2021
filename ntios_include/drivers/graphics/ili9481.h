
#ifndef ILI9481_H
#define ILI9481_H

#include "graphics.h"
#include "../bus8080.h"
#include "../../drivers.h"
#include "../../ntios.h"

class ILI9481: public GraphicsDisplayDevice {
public:
	ILI9481(Bus8080* bus8080, GPIODevice* gpio, int rst, int im0, int im1, int im2, bool inverted = false);

	// Print implementation
	size_t write(uint8_t val);
	using Print::write; // pull in write(str) and write(buf, size) from Print
	void flush();

	const char* getName() { return "ILI9481 Display"; }

	int setTextCursor(int x, int y);
	uint32_t getTextLines();
	uint32_t getTextColumns();

	void clearScreen(uint16_t color);
	void setPixel(int x, int y, uint16_t color);
	//void fillRect(int x1, int y1, int x2, int y2, uint16_t color);
	
	void drawBitmap16(int x, int y, int w, int h, uint16_t* data);

protected:
	void write_command(const uint8_t* data, uint8_t len);
	void write_command(uint8_t cmd);
	void send_sequence(const uint8_t* sequence);

	inline void set_column_adr(uint16_t start, uint16_t end) {
		if (inverted) {
			start = 320 - start;
			end = 320 - end;
		}
		if (start > end) {
			uint16_t tmp = end;
			end = start;
			start = tmp;
		}
		uint8_t data[5] = {
			0x2A,
			(uint8_t)(start >> 8),
			(uint8_t)(start & 255),
			(uint8_t)(end >> 8),
			(uint8_t)(end & 255)
		};

		write_command(data, 5);
	}

	inline void set_page_adr(uint16_t start, uint16_t end) {
		if (inverted) {
			start = 480 - start;
			end = 480 - end;
		}
		if (start > end) {
			uint16_t tmp = end;
			end = start;
			start = tmp;
		}
		uint8_t data[5] = {
			0x2B,
			(uint8_t)(start >> 8),
			(uint8_t)(start & 255),
			(uint8_t)(end >> 8),
			(uint8_t)(end & 255)
		};

		write_command(data, 5);
	}

	int cursor_x;
	int cursor_y;
	Bus8080* bus;
	bool inverted;
};

#endif
