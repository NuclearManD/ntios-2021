
#include "audio_codec.h"

#define BUFFER_SIZE 128

AudioEncoderNAF::AudioEncoderNAF(NTIOSFile* file, uint16_t sample_rate, uint8_t n_channels, uint8_t compression) :
	file {file},
	sample_rate {sample_rate},
	n_channels {n_channels},
	compression {compression}
{
	file->print(" NAF");
	file->write((uint8_t)(NAF_VERSION >> 8));
	file->write((uint8_t)(NAF_VERSION & 255));
	file->write((uint8_t)(sample_rate >> 8));
	file->write((uint8_t)(sample_rate & 255));
	file->write(n_channels);
	file->write(compression);
}

void AudioEncoderNAF::encode(uint16_t* pcm_data, uint16_t num_samples) {
	uint8_t buffer[BUFFER_SIZE];

	if (compression == NAF_COMPRESSION_ADAPTIVE_BITS) {
		uint8_t o = 0;
		uint8_t offset = (uint8_t)(pcm_data[0] >> 8);
		for (int i = 0; i < num_samples; i++) {
			uint8_t high = (uint8_t)(pcm_data[i] >> 8);
			if (high != offset) {
				if (o != 0) {
					file->write(offset);
					file->write(o);
					file->write(buffer, o);
					o = 0;
				}
				offset = high;
			}
			buffer[o++] = (uint8_t)(pcm_data[i] & 255);
			if (o == BUFFER_SIZE) {
				file->write(offset);
				file->write(o);
				file->write(buffer, o);
				o = 0;
			}
		}
		if (o != 0) {
			file->write(offset);
			file->write(o);
			file->write(buffer, o);
		}
	} else if (compression == NAF_NO_COMPRESSION) {
		file->write((uint8_t)(num_samples >> 8));
		file->write((uint8_t)(num_samples & 255));
		int o = 0;
		for (int i = 0; i < num_samples; i++) {
			buffer[o] = (uint8_t)(pcm_data[i] >> 8);
			buffer[++o] = (uint8_t)(pcm_data[i] & 255);
			if ((++o) == BUFFER_SIZE) {
				file->write(buffer, o);
				o = 0;
			}
		}
		if (o != 0)
			file->write(buffer, o);
	}
}

void AudioEncoderNAF::markTime(uint64_t unix_time) {
	uint8_t buffer[12];
	buffer[0] = 0; // set length and offset (if applies) to zero, indicating an attribute mid-recording
	buffer[1] = 0;
	buffer[2] = (uint8_t)(NAF_ATTR_UNIX_TIME >> 8);
	buffer[3] = (uint8_t)NAF_ATTR_UNIX_TIME;
	for (int i = 4; i < 12; i++)
		buffer[i] = (uint8_t)((unix_time >> ((11 - i) * 8)) & 255);
	file->write(buffer, 12);
}

void AudioEncoderNAF::flush() {
	file->flush();
}
