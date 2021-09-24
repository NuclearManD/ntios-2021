
#ifndef IMAGE_CODEC
#define IMAGE_CODEC

#include "../ntios.h"
#include "stdint.h"

#define COMPRESSION_TYPE_NONE 0
#define COMPRESSION_TYPE_DEFLATE 1
#define COMPRESSION_TYPE_RLE8 2
#define COMPRESSION_TYPE_RLE4 3

class ImageDecoder {
public:
	virtual const char* typeName() = 0;
	virtual uint16_t* create16BitBitmap() = 0;
	virtual uint16_t getPixel16Bits() = 0;
	virtual uint32_t getPixel24Bits() = 0;

	virtual int getWidth() = 0;
	virtual int getHeight() = 0;

	virtual uint16_t getCompressionType() = 0;
	virtual uint16_t getBitsPerPixel() = 0;

	virtual void rewind() = 0;
};

class ImageDecoderPNG: public ImageDecoder {
private:

	NTIOSFile* file;

	int width, height;

public:
	ImageDecoderPNG(NTIOSFile* file);

	inline const char* typeName() { return "PNG"; }

	uint16_t* create16BitBitmap();
	uint16_t getPixel16Bits();
	uint32_t getPixel24Bits();

	int getWidth() { return width; }
	int getHeight() { return height; }

	uint16_t getCompressionType() { return COMPRESSION_TYPE_DEFLATE; }
	uint16_t getBitsPerPixel() { return 0; }

	void rewind();
};

class ImageDecoderBMP: public ImageDecoder {
private:

	NTIOSFile* file;

	int width, height;
	int next_column = -1;
	char padsize;
	uint32_t image_start;
	uint16_t bits_per_pixel;
	uint32_t compression_method;
public:
	ImageDecoderBMP(NTIOSFile* file);

	inline const char* typeName() { return "BMP"; }

	uint16_t* create16BitBitmap();
	uint16_t getPixel16Bits();
	uint32_t getPixel24Bits();

	int getWidth() { return width; }
	int getHeight() { return height; }

	inline uint16_t getCompressionType() {
		// The compression method needs to be converted from BMP to NTIOS values.
		switch (compression_method) {
			case 0:
			case 11:
				return COMPRESSION_TYPE_NONE;
			case 1:
			case 12:
				return COMPRESSION_TYPE_RLE8;
			case 2:
			case 13:
				return COMPRESSION_TYPE_RLE4;
			default:
				return COMPRESSION_TYPE_NONE;
		};
	}

	uint16_t getBitsPerPixel() { return bits_per_pixel; }

	void rewind();
};

ImageDecoder* decodeImage(NTIOSFile* file);



#endif
