#include "WAVReader.h"
#include "SampleBuffer.h"
#include "RIFF.h"
#include "WinTypes.h"


WAVReader::WAVReader(std::string path)
	: file(path)
{
	read_info();
}


void WAVReader::read_info()
{
	is_valid = false;

	// Find where the samples are.
	file.seekg(0);
	if (!file.good())
		return;
	char fourcc[4];
	file.read(&fourcc[0], sizeof(fourcc));
	if (!file.good() || !fourcc_eq(fourcc, "RIFF"))
		return;
	auto uber_chunk_size = read_dword();
	if (!file.good())
		return;
	file_end = RIFFChunk::header_size + uber_chunk_size;
	file.read(&fourcc[0], sizeof(fourcc));
	if (!file.good() || !fourcc_eq(fourcc, "WAVE"))
		return;
	auto data_chunk_size = seek_chunk("data");
	if (data_chunk_size <= 0)
		return;
	samples_offset = file.tellg();
	if (!file.good())
		return;

	// Read info from the "fmt " chunk.
	long chunk_size = seek_chunk("fmt ");
	if (chunk_size <= 0)
		return;
	int16_t format_tag = read_word();
	if (!file.good() || format_tag != WAVE_FORMAT_PCM)
		return;
	num_channels = read_word();
	if (!file.good())
		return;
	sample_rate = read_dword();
	if (!file.good())
		return;
	// We don't care about "nAvgBytesPerSec" or "nBlockAlign", skip them.
	read_dword();
	if (!file.good())
		return;
	read_word();
	if (!file.good())
		return;
	bits_per_sample = read_word();
	if (!file.good() || (bits_per_sample != 16 && bits_per_sample != 24))
		return;

	num_samples =
		data_chunk_size / ((bits_per_sample / 8) * num_channels);

	is_valid = true;
}


bool WAVReader::read_samples_into(uint64_t start, uint64_t num_samples, SampleBuffer* buffer)
{
	auto bytes_per_sample = bits_per_sample / 8;
	auto offset = samples_offset + start * num_channels * bytes_per_sample;
	file.seekg(offset);
	if (!file.good())
		return false;

	file.read((char*) buffer->channel_start(0), num_samples * bytes_per_sample * num_channels);
	if (!file.good())
		return false;

	return true;
}


uint32_t WAVReader::num_loops()
{
	auto chunk_start = seek_chunk("smpl");
	if (chunk_start <= 0)
		return 0;
	file.seekg(7 * 4, std::ios_base::cur);
	if (!file.good())
		return 0;
	uint32_t num_loops = read_dword();
	if (!file.good())
		return 0;
	return num_loops;
}


WAVReader::Loop WAVReader::loop(uint32_t index)
{
	auto chunk_start = seek_chunk("smpl");
	if (chunk_start <= 0)
		return {};
	file.seekg(9 * 4 + index * 6 * 4 + 2 * 4, std::ios_base::cur);
	if (!file.good())
		return {};
	Loop loop;
	loop.start = read_dword();
	if (!file.good())
		return {};
	loop.end = read_dword();
	if (!file.good())
		return {};
	return loop;
}


int32_t WAVReader::read_dword()
{
	int32_t raw_dword;
	file.read((char*) &raw_dword, sizeof(raw_dword));
	const uint8_t* data = (const uint8_t*) &raw_dword;
	uint32_t result =
		((uint32_t) data[3]) << 24 |
		((uint32_t) data[2]) << 16 |
		((uint32_t) data[1]) << 8 |
		((uint32_t) data[0]);
	return (int32_t) result;
}

int16_t WAVReader::read_word()
{
	int16_t raw_word;
	file.read((char*) &raw_word, sizeof(raw_word));
	const uint8_t* data = (const uint8_t*) &raw_word;
	uint16_t result =
		((uint16_t) data[1]) << 8 |
		((uint16_t) data[0]);
	return (int16_t) result;
}


long WAVReader::seek_chunk(const char* fourcc)
{
	long position = RIFFChunk::header_size + 4;
	file.seekg(position);
	if (!file.good())
		return -1;

	while (position < file_end) {
		// Read the next chunk header.
		char chunk_fourcc[4];
		file.read(chunk_fourcc, sizeof(chunk_fourcc));
		if (!file.good())
			return -1;
		auto chunk_size = read_dword();
		position += RIFFChunk::header_size;

		// Is this the one?
		if (fourcc_eq(chunk_fourcc, fourcc))
			return chunk_size;

		// Keep looking.
		position += chunk_size;
		file.seekg(position);
		if (!file.good())
			return -1;
		}

	return -1;
}




