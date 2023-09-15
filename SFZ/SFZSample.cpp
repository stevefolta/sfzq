#include "SFZSample.h"
#include "SampleBuffer.h"
#include "WAVReader.h"
#include <iostream>



bool SFZSample::load()
{
	WAVReader reader(path);
	if (!reader.valid())
		return false;
	sample_rate = reader.sample_rate;
	num_samples = reader.num_samples;

	// Read some extra samples, which will be filled with zeros, so interpolation
	// can be done without having to check for the edge all the time.
	buffer =
		new SampleBuffer(
			reader.num_channels, num_samples + 4,
			reader.bits_per_sample, SampleBuffer::Little, SampleBuffer::Planar);
	reader.read_samples_into(0, num_samples, buffer);
	auto num_loops = reader.num_loops();
	if (num_loops > 0) {
		auto loop = reader.loop(0);
		loop_start = loop.start;
		loop_end = loop.end;
		}
	return true;
}


SFZSample::~SFZSample()
{
	delete buffer;
}


std::string SFZSample::short_name()
{
	return path.substr(path.find_last_of('/') + 1);
}


void SFZSample::set_buffer(SampleBuffer* new_buffer)
{
	buffer = new_buffer;
	num_samples = buffer->num_samples;
}


SampleBuffer* SFZSample::detach_buffer()
{
	auto result = buffer;
	buffer = nullptr;
	return result;
}


void SFZSample::dump()
{
	std::cout << path << std::endl;
}


