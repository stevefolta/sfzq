#pragma once

#include "clap/stream.h"
#include <string>

// Wrappers over CLAP streams to handle endianness.

class CLAPInStream {
	public:
		CLAPInStream(const clap_istream_t* stream_in)
			: stream(stream_in) {}

		bool ok = true;

		uint32_t read_uint32();
		double read_double();
		std::string read_string();

	protected:
		const clap_istream_t* stream;
	};


class CLAPOutStream {
	public:
		CLAPOutStream(const clap_ostream_t* stream_in)
			: stream(stream_in) {}

		bool ok = true;

		bool write_uint32(uint32_t value);
		bool write_double(double value);
		bool write_string(std::string str);

	protected:
		const clap_ostream_t* stream;
	};


