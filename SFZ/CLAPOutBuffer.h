#pragma once

#include "OutBuffer.h"
#include "clap/clap.h"


class CLAPOutBuffer : public OutBuffer {
	public:
		CLAPOutBuffer(clap_audio_buffer_t* clap_buffer_in)
			: clap_buffer(clap_buffer_in) {}

		float* samples_for_channel(int channel) {
			return clap_buffer->data32[channel];
			}
		uint32_t num_channels() {
			return clap_buffer->channel_count;
			}

		clap_audio_buffer_t* clap_buffer;
	};

