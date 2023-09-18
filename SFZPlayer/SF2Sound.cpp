#include "SF2Sound.h"
#include "SF2Reader.h"
#include "SFZSample.h"
#include "SampleBuffer.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string_view>


SF2Sound::SF2Sound(std::string path)
	: SFZSound(path)
{
}


SF2Sound::~SF2Sound()
{
	// "presets" owns the regions, so clear them out of "regions" so ~SFZSound()
	// doesn't try to delete them.
	regions.clear();

	// The samples all share a single buffer, so make sure they don't all delete it.
	SampleBuffer* buffer = nullptr;
	for (auto& kv: samples_by_rate)
		buffer = kv.second->detach_buffer();
	delete buffer;
}


void SF2Sound::load_regions()
{
	SF2Reader reader(this, path);
	reader.read();

	// Sort the presets.
	std::sort(presets.begin(), presets.end(), [](const SF2Sound::Preset* first, const SF2Sound::Preset* second) {
		if (first->bank < second->bank)
			return true;
		else if (first->bank > second->bank)
			return false;
		else
			return first->preset < second->preset;
		});

	use_subsound(0);
}


void SF2Sound::load_samples(std::function<void(double)> progress_fn)
{
	SF2Reader reader(this, path);
	SampleBuffer* buffer = reader.read_samples(progress_fn);
	if (buffer) {
		// All the SFZSamples will share the buffer.
		for (auto& kv: samples_by_rate)
			kv.second->set_buffer(buffer);
		}

	if (progress_fn)
		progress_fn(1.0);
}


void SF2Sound::add_preset(SF2Sound::Preset* preset)
{
	presets.push_back(preset);
}


int SF2Sound::num_subsounds()
{
	return presets.size();
}


static void put_utf8(uint32_t c, std::ostringstream& out)
{
	int bytes_left = 0;
	if (c < 0x80)
		out << c;
	else if (c < 0x800) {
		out << (0xC0 | (c >> 6));
		bytes_left = 1;
		}
	else if (c < 0x10000) {
		out << (0xE0 | (c >> 12));
		bytes_left = 2;
		}
	else if (c < 200000) {
		out << (0xF0 | (c >> 18));
		bytes_left = 3;
		}
	else if (c < 0x04000000) {
		out << (0xF8 | (c >> 24));
		bytes_left = 4;
		}
	else {
		out << (0xFC | (c >> 30));
		bytes_left = 5;
		}

	int shift = (bytes_left - 1) * 6;
	while (bytes_left-- > 0) {
		out << (0x80 | ((c >> shift) & 0x3F));
		shift -= 6;
		}
}

static std::string convert_8859_1_to_utf8(std::string_view in)
{
	static uint32_t cp_1252_chars[32] = {
		0x20AC, 0, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
		0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0, 0x017D, 0,
		0, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
		0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0, 0x017E, 0x0178,
		};

	std::ostringstream result;
	for (uint8_t c: in) {
		if (c < 0x80)
			result << c;
		else if (c < 0xA0)
			put_utf8(cp_1252_chars[c - 0x80], result);
		else
			put_utf8(c, result);
		}
	return result.str();
}

std::string SF2Sound::subsound_name(int which_subsound)
{
	Preset* preset = presets[which_subsound];
	std::ostringstream result;
	if (preset->bank != 0)
		result << preset->bank << "/";
	result << preset->preset << ": " << convert_8859_1_to_utf8(preset->name);
	return result.str();
}


void SF2Sound::use_subsound(int which_subsound)
{
	if (which_subsound >= (int) presets.size())
		return;

	selected_preset = which_subsound;
	regions = presets[which_subsound]->regions;
}


int SF2Sound::selected_subsound()
{
	return selected_preset;
}


SFZSample* SF2Sound::sample_for(unsigned long sample_rate)
{
	SFZSample* sample = samples_by_rate[sample_rate];
	if (sample == nullptr) {
		sample = new SFZSample(sample_rate);
		samples_by_rate[sample_rate] = sample;
		}
	return sample;
}


void SF2Sound::set_samples_buffer(SampleBuffer* buffer)
{
	for (auto& kv: samples_by_rate)
		kv.second->set_buffer(buffer);
}



