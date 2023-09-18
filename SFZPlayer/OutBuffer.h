#pragma once


class OutBuffer {
	public:
		virtual float* samples_for_channel_32(int channel) = 0;
		virtual double* samples_for_channel_64(int channel) { return nullptr; }
		virtual uint32_t num_channels() = 0;
	};

