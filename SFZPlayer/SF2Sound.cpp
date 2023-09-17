#include "SF2Sound.h"
#include "SF2Reader.h"
#include "SFZSample.h"
#include "SampleBuffer.h"
#include <algorithm>


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


void SF2Sound::load_samples(double* progress_var)
{
	SF2Reader reader(this, path);
	SampleBuffer* buffer = reader.read_samples(progress_var);
	if (buffer) {
		// All the SFZSamples will share the buffer.
		for (auto& kv: samples_by_rate)
			kv.second->set_buffer(buffer);
		}

	if (progress_var)
		*progress_var = 1.0;
}


void SF2Sound::add_preset(SF2Sound::Preset* preset)
{
	presets.push_back(preset);
}


int SF2Sound::num_subsounds()
{
	return presets.size();
}


std::string SF2Sound::subsound_name(int which_subsound)
{
	Preset* preset = presets[which_subsound];
	std::string result;
	if (preset->bank != 0) {
		result += preset->bank;
		result += "/";
		}
	result += preset->preset;
	result += ": ";
	result += preset->name;
	return result;
}


void SF2Sound::use_subsound(int which_subsound)
{
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



