#pragma once

#include <vector>
#include <stdint.h>
#include <stddef.h>


class SampleBuffer {
	public:
		enum Endianness {
			Big, Little,
			};
		enum Layout {
			Planar, Interleaved,
			};
		typedef float (*ReadFn)(const uint8_t*);

		SampleBuffer(int num_channels, int num_samples, int bits_per_sample, Endianness endianness, Layout layout);
		bool valid() {
			return read_sample != nullptr;
			}

		int num_channels, num_samples;
		std::vector<uint8_t> sample_data;

		ptrdiff_t stride = 0;
		ptrdiff_t channel_offset = 0;
		ReadFn read_sample = nullptr;
		uint8_t* channel_start(int channel) { return sample_data.data() + channel * channel_offset; }
		uint8_t* channel_end(int channel) { return channel_start(channel) + channel_offset; }

		/* How to use:
			 auto in_read = in_buffer->read_sample;
			 auto in_p = in_buffer->channel_start(which_channel) + start_sample * in_buffer->stride;
			 while (...) {
			 	float sample = in_read(in_p);
				in_p += in_buffer->stride;
				 }
		*/
	};


