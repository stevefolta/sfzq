#pragma once

#include <vector>


class SampleBuffer {
	public:
		SampleBuffer(int num_channels_in, int num_samples_in)
			: num_channels(num_channels_in), num_samples(num_samples_in),
			samples(num_channels_in * num_samples_in)
		{
		}

		int num_channels, num_samples;
		std::vector<float> samples;
	};

