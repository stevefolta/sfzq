#include "CLAPStream.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

// Values are stored in network-endian (big-endian) format.

static bool is_little_endian;
static bool supports_doubles;

class CLAPStreamInitializer {
	public:
		CLAPStreamInitializer() {
			int value = 1;
			is_little_endian = ((char*) &value)[0] != 0;
			supports_doubles = (sizeof(double) == 8);
			}
	};
static CLAPStreamInitializer initializer;


static uint32_t swap_uint32(uint32_t value)
{
	return
		value >> 24 |
		((value >> 8) & 0xFF00) |
		(value & 0xFF00) << 8 |
		value << 24;
}

static double swap_double(double value)
{
	uint8_t* bytes = (uint8_t*) &value;
	for (unsigned int i = 0; i < sizeof(double) / 2; ++i) {
		uint8_t tmp = bytes[sizeof(double) - i - 1];
		bytes[sizeof(double) - i - 1] = bytes[i];
		bytes[i] = tmp;
		}
	return value;
}


uint32_t CLAPInStream::read_uint32()
{
	uint32_t value = 0;
	int64_t bytes_read = stream->read(stream, &value, sizeof(value));
	if (bytes_read != sizeof(value))
		ok = false;
	if (is_little_endian)
		value = swap_uint32(value);
	return value;
}

double CLAPInStream::read_double()
{
	double value = 0.0;
	int64_t bytes_read = stream->read(stream, &value, sizeof(value));
	if (bytes_read != sizeof(value) || !supports_doubles)
		ok = false;
	if (is_little_endian)
		value = swap_double(value);
	return value;
}

std::string CLAPInStream::read_string()
{
	std::string result;
	auto size = read_uint32();
	if (!ok)
		return result;
	char* buffer = (char*) malloc(size);
	uint32_t bytes_left = size;
	char* p = buffer;
	while (ok && bytes_left > 0) {
		auto bytes_read = stream->read(stream, p, bytes_left);
		if (bytes_read < 0) {
			ok = false;
			break;
			}
		bytes_left -= bytes_read;
		p += bytes_read;
		}
	if (ok)
		result.assign(buffer, size);
	free(buffer);
	return result;
}


bool CLAPOutStream::write_uint32(uint32_t value)
{
	if (is_little_endian)
		value = swap_uint32(value);
	int64_t bytes_written = stream->write(stream, &value, sizeof(value));
	if (bytes_written != sizeof(value)) {
		ok = false;
		return false;
		}
	return true;
}

bool CLAPOutStream::write_double(double value)
{
	if (is_little_endian)
		value = swap_double(value);
	int64_t bytes_written = stream->write(stream, &value, sizeof(value));
	if (bytes_written != sizeof(value) || !supports_doubles) {
		ok = false;
		return false;
		}
	return true;
}


bool CLAPOutStream::write_string(std::string str)
{
	uint32_t bytes_left = str.size();
	if (!write_uint32(bytes_left))
		return false;
	auto p = str.data();
	while (bytes_left > 0) {
		auto bytes_written = stream->write(stream, p, bytes_left);
		if (bytes_written < 0)
			return false;
		bytes_left -= bytes_written;
		p += bytes_written;
		}
	return true;
}



