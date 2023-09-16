#pragma once

#include "WinTypes.h"
#include <string>
#include <istream>
#include <stdint.h>


struct RIFFChunk {
	enum Type {
		RIFF,
		LIST,
		Custom
		};

	enum {
		header_size = 8,
		};

	fourcc id;
	dword size;
	Type type;
	int64_t start;

	void read_from(std::istream* file);
	void seek(std::istream* file);
	void seek_after(std::istream* file);
	int64_t end() { return start + size; }

	std::string read_string(std::istream* file);
	};


