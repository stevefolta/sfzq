#pragma once

#include <string>
#include <stdint.h>

class SampleBuffer;


class SFZSample {
	public:
		SFZSample(std::string path_in)
			: path(path_in) {}
		SFZSample(double sample_rate_in)
			: sample_rate(sample_rate_in) {}
		~SFZSample();

		bool load();
		std::string short_name();
		void set_buffer(SampleBuffer* new_buffer);
		SampleBuffer* detach_buffer();
		void dump();

		SampleBuffer* buffer = nullptr;
		uint64_t num_samples = 0, loop_start = 0, loop_end = 0;
		double sample_rate;

	protected:
		std::string path;
	};

