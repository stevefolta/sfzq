#pragma once

#include <string>
#include <stdint.h>
#include <fstream>

class SampleBuffer;


class WAVReader {
	public:
		WAVReader(std::string path);

		double sample_rate = 0;
		int num_channels = 0;
		int num_samples = 0;
		int bits_per_sample = 0;

		bool valid() { return is_valid; }
		bool read_samples_into(uint64_t start, uint64_t num_samples, SampleBuffer* buffer);

		struct Loop {
			uint64_t start, end;
			};
		uint32_t num_loops();
		Loop loop(uint32_t index);

	protected:
		enum {
			WAVE_FORMAT_PCM = 1,
			};

		std::fstream file;
		bool is_valid;
		long samples_offset, file_end;

		void read_info();
		int32_t read_dword();
		int16_t read_word();
		long seek_chunk(const char* fourcc);
	};


