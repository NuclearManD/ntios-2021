
#include "audio_codec.h"

/*
class AudioDecoderMP3 {
private:
	NTIOSFile* file;
	uint32_t sample_rate;
	uint32_t bit_rate;
	bool padded;
	uint8_t mode;

const uint8_t _mp3_bitrates[16] = {
	0,
	32,
	40,
	48,
	56,
	64,
	80,
	96,
	112,
	128,
	160,
	192,
	224,
	256,
	320,
	1
};

const uint16_t _mp3_sample_rates[4] = {
	441,
	480,
	320,
	0
};

AudioDecoderMP3::AudioDecoderMP3(NTIOSFile* file) {
	char header[4];

	file->read(header, 4);
	if (header[0] != 0xFF || header[1] != 0xBF)
		return;

	bit_rate = _mp3_bitrates[header[2] & 15] * 1000;
	sample_rate = _mp3_sample_rates[(header[2] >> 4) & 3] * 100;

	padded = (header[2] >> 6) & 1;
	mode = header[3];

	if (bit_rate < 32000 || sample_rate < 10000)
		return;

	this->file = file;
	
	frame_len = (144 * bit_rate) / sample_rate + ((header[2] >> 6) & 1);
}

int AudioDecoderMP3::decode(uint16_t* pcm_data, uint16_t read_size) {
	if (file == nullptr) return -1;
}
*/

AudioDecoderNAF::AudioDecoderNAF(NTIOSFile* file) {
	char buffer[10];
	file->read(buffer, 10);

	if (memcmp(buffer, " NAF", 4))
		return;

	if (((buffer[4] << 8) || buffer[5]) > NAF_VERSION)
		return;

	sample_rate = ((buffer[6] << 8) || buffer[7]);
	n_channels = buffer[8];
	compression = buffer[9];
	this->file = file;
}

uint16_t AudioDecoderNAF::decode(uint16_t* pcm_data, uint16_t max_samples) {

	if (file->available() == 0)
		return 0;

	if (compression == NAF_COMPRESSION_ADAPTIVE_BITS) {
		
		/*uint8_t o = 0;
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
		}*/

		uint32_t samples_read = 0;
		while (file->available() && samples_read + 4 < max_samples) {

			// Attempt to get a group
			int pos = file->position();
			uint8_t high = file->read();
			uint8_t group_size = file->read();

			// If the group would overflow our buffer, then return
			if (group_size + samples_read > max_samples) {
				file->seek(pos);
				break;
			}

			// Read the group
			for (int i = 0; i < group_size; i++)
				pcm_data[samples_read++] = (high << 8) | file->read();

		}

		return (uint16_t)samples_read;

	} else if (compression == NAF_NO_COMPRESSION) {
		uint16_t num_samples = (file->read() << 8) | file->read();
		if (num_samples > max_samples)
			return false;

		for (uint32_t i = 0; i < num_samples; i++)
			pcm_data[i] = (file->read() << 8) | file->read();

		return num_samples;
	}
}

AudioDecoder* decodeAudio(NTIOSFile* file) {
	AudioDecoder* decoder;

	/*// Attempt MP3 format
	decoder = new AudioDecoderMP3(file);
	if (!*decoder)
		delete decoder;
	else
		return decoder;*/

	// Attempt NAF format
	decoder = new AudioDecoderNAF(file);
	if (!*decoder)
		delete decoder;
	else
		return decoder;

	// No decoders worked, return failure
	return nullptr;
}
