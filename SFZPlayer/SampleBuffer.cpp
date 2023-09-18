#include "SampleBuffer.h"


SampleBuffer::SampleBuffer(
	int num_channels_in, int num_samples_in,
	int bits_per_sample, SampleBuffer::Endianness endianness, SampleBuffer::Layout layout)
	: num_channels(num_channels_in), num_samples(num_samples_in),
	sample_data(num_channels_in * num_samples_in * (bits_per_sample / 8))
{
	stride = bits_per_sample / 8;
	if (layout == Interleaved) {
		channel_offset = stride;
		stride *= num_channels;
		}
	else
		channel_offset = num_samples * stride;

	if (endianness == Little) {
		if (bits_per_sample == 16) {
			read_sample = [](const uint8_t* p) -> sfz_float {
				uint16_t usample = (uint16_t) p[1] << 8 | p[0];
				return ((int16_t) usample) / (sfz_float) INT16_MAX;
				};
			}
		else if (bits_per_sample == 24) {
			read_sample = [](const uint8_t* p) -> sfz_float {
				uint32_t usample = (uint32_t) p[2] << 24 | p[1] << 16 | p[0] << 8;
				return ((int32_t) usample) / (sfz_float) INT32_MAX;
				};
			}
		}
	else /* endianness == Big */ {
		if (bits_per_sample == 16) {
			read_sample = [](const uint8_t* p) -> sfz_float {
				uint16_t usample = (uint16_t) p[0] << 8 | p[1];
				return ((int16_t) usample) / (sfz_float) INT16_MAX;
				};
			}
		else if (bits_per_sample == 24) {
			read_sample = [](const uint8_t* p) -> sfz_float {
				uint32_t usample = (uint32_t) p[0] << 24 | p[1] << 16 | p[0] << 8;
				return ((int32_t) usample) / (sfz_float) INT32_MAX;
				};
			}
		}
}


