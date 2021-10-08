
#ifndef AUDIO_ENCODER_H
#define AUDIO_ENCODER_H

#include "../ntios.h"
#include "stdint.h"

// Version 1 added compression
// Version 2 added attributes
#define NAF_VERSION 0x0002

#define NAF_NO_COMPRESSION 0
// lossless audio compression that changes the number of bits per sample - works good for
// audio files which have a good amount of really quiet audio that shouldn't be lost
// Theoretical maximum compression vs. no compression: ~50%
// Reasonable maximum compression vs. no compression: 75-80% if the file has a good amount of very quiet audio
#define NAF_COMPRESSION_ADAPTIVE_BITS 1

// time attribute allows the program reading the file to correct for the audio file length 'drifting'
// as the audio is recorded a few samples may get missed, or the sample rate marked in the file doesn't
// *exactly* match the real sample rate.  For a large audio file the errors may add up, so metadata can
// keep the audio recording's "clock" synchronized to real time.
#define NAF_ATTR_UNIX_TIME 0


class AudioEncoderNAF {
private:
	NTIOSFile* file;
	uint16_t sample_rate;
	uint8_t n_channels;
	uint8_t compression;

public:
	AudioEncoderNAF(NTIOSFile* file, uint16_t sample_rate, uint8_t n_channels, uint8_t compression = NAF_NO_COMPRESSION);

	void encode(uint16_t* pcm_data, uint16_t num_samples);
	void flush();
	void markTime(uint64_t unix_time);
};


class AudioDecoder {
public:
	virtual uint16_t decode(uint16_t* pcm_data, uint16_t read_size) = 0;
	
	bool operator!() {
		return !((bool)*this);
	}

	virtual operator bool() = 0;

	virtual bool available() = 0;

	virtual ~AudioDecoder() {}
};

/*
class AudioDecoderMP3: public AudioDecoder {
private:
	NTIOSFile* file = nullptr;
	uint32_t sample_rate;
	uint32_t bit_rate;
	bool padded;
	uint8_t mode;
	uint16_t frame_len;
public:
	AudioDecoderMP3(NTIOSFile* file);
	int decode(uint16_t* pcm_data, uint16_t read_size);

	operator bool() { return file != nullptr; }

};*/


class AudioDecoderNAF: public AudioDecoder {
private:
	NTIOSFile* file = nullptr;
	uint16_t sample_rate;
	uint8_t n_channels;
	uint8_t compression;

public:
	AudioDecoderNAF(NTIOSFile* file);
	uint16_t decode(uint16_t* pcm_data, uint16_t max_samples);
	bool available() { return file->available(); }

	operator bool() { return file != nullptr; }

	~AudioDecoderNAF();
};


AudioDecoder* decodeAudio(NTIOSFile* file);

#endif
