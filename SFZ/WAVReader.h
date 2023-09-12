#pragma once

#include <string>
#include <stdint.h>

class SampleBuffer;


class WAVReader {
	public:
		WAVReader(std::string path_in);

		bool valid();
		double sample_rate();
		int num_channels();
		uint64_t num_samples();
		bool read_samples_into(uint64_t start, uint64_t num_samples, SampleBuffer* buffer);

		struct Loop {
			uint64_t start, end;
			};
		uint32_t num_loops();
		Loop loop(uint32_t index);
	};


