#pragma once

class OutBuffer {
	public:
		virtual float* samples_for_channel(int channel) = 0;
		virtual uint32_t num_channels() = 0;
	};

