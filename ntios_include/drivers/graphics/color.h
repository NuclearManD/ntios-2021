
#ifndef COLOR_H
#define COLOR_H

#include <stdint.h>

/*typedef union {
	uint32_t raw;
	struct {
		uint8_t red;
		uint8_t green;
		uint8_t blue;
		uint8_t zero;
	} rgb;
} color_t;*/
typedef uint32_t color_t;

#define BLACK 0
#define WHITE 0xFFFFFF
#define BRIGHT_RED 0x0000FF
#define BRIGHT_GREEN 0x00FF00
#define BRIGHT_BLUE 0xFF0000
#define DARK_GRAY 0x555555
#define LIGHT_GRAY 0xAAAAAA
#define GRAY 0x808080
#define RED 0x000080
#define GREEN 0x008000
#define BLUE 0x800000
#define PURPLE 0x800080
/*
// color_t to rrrgggbb (LSB first)
static inline uint8_t color_to_uint8(color_t color) {
	return (color.rgb.red >> 5) | ((color.rgb.green >> 2) & 56) | (color.rgb.blue & 192);
}

// color_t to rrrrrggggggbbbbb (LSB first)
static inline uint16_t color_to_uint16(color_t color) {
	return (color.rgb.red >> 3) | ((color.rgb.green << 2) & 0x7e0) | ((color.rgb.blue << 8) & 0xf800);
}*/

// color_t to rrrgggbb (LSB first)
static inline uint8_t color_to_uint8(color_t color) {
	uint8_t red = (color & 0xFF);
	uint8_t green = (color & 0xFF00) >> 8;
	uint8_t blue = (color & 0xFF0000) >> 16;
	return (red >> 5) | ((green >> 2) & 56) | (blue & 192);
}

// color_t to rrrrrggggggbbbbb (LSB first)
static inline uint16_t color_to_uint16(color_t color) {
	uint8_t red = (color & 0xFF);
	uint8_t green = (color & 0xFF00) >> 8;
	uint16_t blue = (color & 0xFF0000) >> 16;
	return (red >> 3) | ((green << 3) & 0x7e0) | ((blue << 8) & 0xf800);
}

static inline uint16_t uint8_color_to_uint16(uint8_t color) {
	uint16_t red = (color & 7) << 5;
	uint16_t green = (color & 56) << 2;
	uint16_t blue = (color & 192);
	return (red >> 3) | ((green << 2) & 0x7e0) | ((blue << 8) & 0xf800);
}

// create rrrrrggggggbbbbb (LSB first) from 8-bit color components
static inline uint16_t make_color_uint16(uint8_t red, uint8_t green, uint8_t blue) {
	return (red >> 3) | ((green << 3) & 0x7e0) | ((blue << 8) & 0xf800);
}

#endif
