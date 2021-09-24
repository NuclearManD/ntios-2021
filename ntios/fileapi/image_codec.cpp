
#include <stdlib.h>
#include "image_codec.h"
#include "../drivers/graphics/color.h"

const uint8_t png_signature[] = { 137, 80, 78, 71, 13, 10, 26, 10 };

typedef struct {
	uint32_t length;
	char name[4];
	uint8_t* data;
	uint32_t crc;
} png_chunk_t;

static inline uint32_t bytesToUint32(const uint8_t* buf) {
	return buf[3] | (buf[2] << 8) | (buf[1] << 16) | (buf[0] << 24);
}

static inline uint32_t bytesToUint32Little(const uint8_t* buf) {
	return buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
}

static png_chunk_t get_png_chunk(NTIOSFile* file) {
	png_chunk_t chunk;
	uint8_t buf[4];

	// Check if we even have data left to read
	if (file->available() < 12) {
		chunk.length = -1;
		return chunk;
	}

	file->read((uint8_t*)buf, 4); // This gets the length, in big endian
	file->read((char*)chunk.name, 4); // Read out the chunk name
	chunk.length = bytesToUint32(buf);

	// Detect not enough bytes
	if (file->available() < chunk.length + 4) {
		chunk.length = -1;
		return chunk;
	}

	// Allocate memory for the data section
	chunk.data = (uint8_t*)malloc(chunk.length);
	if (chunk.data == nullptr) // Detect memory error
		return chunk;

	// Read out the data
	file->read(chunk.data, chunk.length);

	// Read the CRC
	file->read((uint8_t*)buf, 4);
	chunk.crc = bytesToUint32(buf);

	return chunk;
}

ImageDecoderPNG::ImageDecoderPNG(NTIOSFile* file) {
	png_chunk_t first_chunk = get_png_chunk(file);

	this->file = file;
	width = bytesToUint32(first_chunk.data);
	height = bytesToUint32(first_chunk.data + 4);
}

void ImageDecoderPNG::rewind() {
	// Not sure what to do here

	//// Go to the beginning of the color data
	//file->seek(image_start);
}

uint16_t* ImageDecoderPNG::create16BitBitmap() {
	return 0;
}

uint16_t ImageDecoderPNG::getPixel16Bits() {
	return 0;
}

uint32_t ImageDecoderPNG::getPixel24Bits() {
	return 0;
}

ImageDecoderBMP::ImageDecoderBMP(NTIOSFile* file) {
	uint8_t buffer[4];

	this->file = file;

	file->seek(0x0A);

	// Get the start offset of the actual bitmap
	file->read(buffer, 4);
	image_start = bytesToUint32Little(buffer);

	// Get the image size
	file->seek(0x12);
	file->read(buffer, 4);
	width = bytesToUint32Little(buffer);
	file->read(buffer, 4);
	height = bytesToUint32Little(buffer);

	// Get some other data about the image
	file->seek(0x1C);
	bits_per_pixel = file->read() | (file->read() << 8);
	file->read(buffer, 4);
	compression_method = bytesToUint32Little(buffer);

	padsize = ((width * bits_per_pixel) / 8) % 4;
	if (padsize != 0)
		padsize = 4 - padsize;
}

uint16_t* ImageDecoderBMP::create16BitBitmap() {
	uint16_t* bitmap_buffer = (uint16_t*)malloc(2 * width * height);

	if (bitmap_buffer == nullptr)
		return nullptr;

	rewind();

	// Read out every pixel sequentially
	// There's likely a faster way to do this, but this is something to improve
	// another time.
	for (int i = 0; i < width * height; i++)
		bitmap_buffer[i] = getPixel16Bits();

	return bitmap_buffer;
}

uint16_t ImageDecoderBMP::getPixel16Bits() {
	uint16_t out = 0xFFFF;
	uint8_t buf[4];
	
	if (compression_method == 0) {
		switch (bits_per_pixel) {
			case 8:
				out = file->read() << 8;
				break;
			case 16:
				out = file->read() | (file->read() << 8);
				break;
			case 24:
				file->read(buf + 1, 3);
				buf[0] = 0;
				out = color_to_uint16(bytesToUint32(buf));
				break;
			case 32:
				file->read(buf, 4);
				out = color_to_uint16(bytesToUint32(buf));
				break;
			default:
				out = 0xFFFF; // White
				break;
		}
	}

	next_column++;
	if (next_column == width) {
		next_column = 0;
		
		file->read(buf, padsize);
	}

	return out;
}

uint32_t ImageDecoderBMP::getPixel24Bits() {
	uint32_t out = 0xFFFFFF;
	uint8_t buf[4];
	
	if (compression_method == 0) {
		switch (bits_per_pixel) {
			case 8:
				out = file->read() << 8;
				break;
			case 16:
				out = file->read() | (file->read() << 8);
				break;
			case 24:
				file->read(buf + 1, 3);
				buf[0] = 0;
				out = bytesToUint32(buf);
				break;
			case 32:
				file->read(buf, 4);
				out = bytesToUint32(buf);
				break;
			default:
				out = 0xFFFFFF; // White
				break;
		}
	}

	next_column++;
	if (next_column == width) {
		next_column = 0;
		
		file->read(buf, padsize);
	}

	return out;
}

void ImageDecoderBMP::rewind() {
	// Go to the beginning of the color data
	file->seek(image_start);

	// Mark that we're at the beginning of a column
	next_column = 0;
}

ImageDecoder* decodeImage(NTIOSFile* file) {
	uint8_t first8bytes[8];

	if (file == nullptr)
		return nullptr;

	file->read(first8bytes, 8);
	if (memcmp(png_signature, first8bytes, 8) == 0) {
		// Check other PNG stuff
		return new ImageDecoderPNG(file);
	} else if (memcmp(first8bytes, "BM", 2) == 0) {
		// BMP image
		return new ImageDecoderBMP(file);
	}

	// Unrecognized file type
	return nullptr;
}
