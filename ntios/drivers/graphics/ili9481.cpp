
#include <stdint.h>
#include "ili9481.h"

#define COMMAND 0
#define DATA 1

const uint8_t initialization_sequence[] = {
	// Each line here is a command length (including 1 for command byte) followed by the command & data
	// 255 as the length creates a delay for 10 milliseconds
	// 0 as the length ends the sequence

	1, 0x11, // Exit sleep mode
	255, // Delay 20ms
	255,
	2, 0xB0, 0x04,
	255,
	6, 0xB3, 0x02, 0x00, 0x00, 0x30, // Frame memory access/interface settings
	255,
	2, 0xB4, 0x00, // Use internal oscillator
	4, 0xD0, 0x07, 0x42, 0x18, // Power Settings
	4, 0xD1, 0x00, 0x07, 0x10, // Vcom ctrl
	3, 0xD2, 0x01, 0x02, // More power settings
	6, 0xC0, 0x10, 0x3B, 0x00, 0x02, 0x11, // Panel drive settings
	2, 0xC5, 0x02, // 85Hz frame rate, inversion control.  Use 0x03 for 72Hz.
	//17, 0xE0, 0x00, 0x0F, 0x21, 0x1C, 0x0B, 0x0E, 0x08, 0x49, 0x98, 0x38, 0x09, 0x11, 0x03, 0x14, 0x10, 0x00,
	//17, 0xE1, 0x00, 0x0F, 0x2F, 0x2B, 0x0C, 0x0E, 0x06, 0x47, 0x76, 0x37, 0x07, 0x11, 0x04, 0x23, 0x1E, 0x00,
	13, 0xC8, 0, 0xf3, 0, 0xbc, 0x50, 0x1f, 0, 7, 0x7f, 0x7, 0xf, 0,
	1, 0x29, // Turn on!
	255,
	2, 0x36, 0x02, // Set address mode: Horizontal flip, RGB order, normal, left->right, top->bottom
	2, 0x3A, 0x55, // Set pixel format: 16 bit color
	1, 0x21, // Invert the display.  For the test display I'm using, this is needed to UNinvert color.
	0, // Stop sequence
};

void default_font_drawChar(GraphicsDisplayDevice* disp, uint16_t color, int sx, int sy, char c);

ILI9481::ILI9481(Bus8080* bus8080, GPIODevice* gpio, int rst, int im0, int im1, int im2, bool inverted):
	bus(bus8080),
	inverted(inverted)
{
	gpio->pinMode(rst, GPIO_PIN_MODE_OUTPUT);
	gpio->pinMode(im0, GPIO_PIN_MODE_OUTPUT);
	gpio->pinMode(im1, GPIO_PIN_MODE_OUTPUT);
	gpio->pinMode(im2, GPIO_PIN_MODE_OUTPUT);

	// Set the mode
	gpio->writePin(im0, true);
	gpio->writePin(im1, true);
	gpio->writePin(im2, false);

	// Reset the ili9481
	gpio->writePin(rst, true);
	delay(1);
	gpio->writePin(rst, false);
	delay(1);
	gpio->writePin(rst, true);

	delay(5);

	bus->select();
	// Execute initialization sequence
	send_sequence(initialization_sequence);
	bus->deselect();

	clearScreen(0);
}

void ILI9481::send_sequence(const uint8_t* sequence) {
	uint8_t len;
	while (len = *(sequence++)) {
		if (len == 255) {
			delay(10);
		} else {
			write_command(sequence, len);
			sequence += len;
		}
	}
}

void ILI9481::write_command(const uint8_t* data, uint8_t len) {
	bus->write(COMMAND, data, 1);
	bus->write(DATA, data + 1, len - 1);
}

void ILI9481::write_command(uint8_t cmd) {
	bus->write(COMMAND, &cmd, 1);
}

size_t ILI9481::write(uint8_t val) {
	default_font_drawChar(this, 0xFFFF, cursor_x, cursor_y, val);
	cursor_x += 8;
	return 1;
}

void ILI9481::flush() {}

void ILI9481::clearScreen(uint16_t color) {
	uint8_t color_buffer[32];

	// This will allow us to spit out data faster in a sec
	for (int i = 0; i < 16; i++) {
		color_buffer[i*2] = color >> 8;
		color_buffer[i*2 + 1] = color & 255;
	}

	bus->select();

	// Select entire screen
	set_column_adr(0, 320);
	set_page_adr(0, 480);

	// Write entire screen
	write_command(0x2C);
	for (int i = 0; i < 320*480 / 16; i++) {
		bus->write(DATA, color_buffer, 32);
	}

	// Stop the command
	write_command(0x00);

	bus->deselect();
}

void ILI9481::setPixel(int x, int y, uint16_t color) {
	uint8_t buffer[] = {
		0x2C, // Write memory
		(uint8_t)(color >> 8),
		(uint8_t)(color & 255)
	};

	bus->select();

	// Select pixel
	set_column_adr(x, x + 1);
	set_page_adr(y, y + 1);

	// Write pixel
	write_command(buffer, 3);

	// Stop the command
	write_command(0x00);

	bus->deselect();
}

void ILI9481::drawBitmap16(int x, int y, int w, int h, uint16_t* data) {
	bus->select();

	for (int i = 0; i < h; i++) {
		// Select row
		set_column_adr(x, x + w);
		set_page_adr(i + y, i + 1 + y);

		// Write row
		write_command(0x2C);
		bus->write16BigEndian(DATA, &data[w * i], w);

		// Stop the command
		write_command(0x00);
	}

	bus->deselect();
}

/*
// Not sure why this doesn't work
void ILI9481::fillRect(int x1, int y1, int x2, int y2, uint16_t color) {
	int w = x2 - x1;
	int h = y2 - y1;

	bus->select();

	//for (int i = y1; i < y2; i++) {
		// Select row
		set_column_adr(x1, x2);
		set_page_adr(y1, y2);

		// Write row
		write_command(0x2C);
		for (int j = 0; j < w * h; j++)
			bus->write16BigEndian(DATA, &color, 1);

		// Stop the command
		write_command(0x00);
	//}

	bus->deselect();
}
*/
int ILI9481::setTextCursor(int x, int y) {
	cursor_x = x;
	cursor_y = y;
	return 0;
}

int ILI9481::getTextLines() {
	return 0;
}

int ILI9481::getTextColumns() {
	return 0;
}
