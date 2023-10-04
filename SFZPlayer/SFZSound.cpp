#include "SFZSound.h"
#include "SFZRegion.h"
#include "SFZSample.h"
#include "SFZReader.h"
#include <sstream>
#include <iostream>



SFZSound::SFZSound(std::string path_in)
	: path(path_in)
{
}


SFZSound::~SFZSound()
{
	int num_regions = regions.size();
	for (int i = 0; i < num_regions; ++i) {
		delete regions[i];
		regions[i] = nullptr;
		}

	for (auto& kv: samples)
		delete kv.second;
}


void SFZSound::add_region(SFZRegion* region)
{
	regions.push_back(region);
}


SFZSample* SFZSound::add_sample(std::string sample_path, std::string default_path)
{
	sample_path = fix_slashes(sample_path);
	default_path = fix_slashes(default_path);
	if (default_path.empty())
		sample_path = sibling_path(path, sample_path);
	else {
		std::string default_dir = sibling_path(path, default_path);
		sample_path = default_dir + "/" + sample_path;
		}
	SFZSample* sample = samples[sample_path];
	if (sample == nullptr) {
		sample = new SFZSample(sample_path);
		samples[sample_path] = sample;
		}
	return sample;
}


void SFZSound::add_error(const std::string& message)
{
	errors.push_back(message);
}


void SFZSound::add_unsupported_opcode(const std::string& opcode)
{
	unsupported_opcodes[opcode] = opcode;
}


void SFZSound::load_regions()
{
	SFZReader reader(this);
	reader.read(path);
}


void SFZSound::load_samples(std::function<void(double)> progress_fn)
{
	if (progress_fn)
		progress_fn(0.0);

	double num_samples_loaded = 1.0, num_samples = samples.size();
	for (const auto& kv: samples) {
		auto sample = kv.second;
		bool ok = sample->load();
		if (!ok)
			add_error("Couldn't load sample \"" + sample->short_name() + "\"");

		num_samples_loaded += 1.0;
		if (progress_fn)
			progress_fn(num_samples_loaded / num_samples);
		}

	if (progress_fn)
		progress_fn(1.0);
}


SFZRegion* SFZSound::get_region_for(
	int note, int velocity, float rand_val, SFZRegion::Trigger trigger)
{
	int num_regions = regions.size();
	for (int i = 0; i < num_regions; ++i) {
		SFZRegion* region = regions[i];
		if (region->matches(note, velocity, trigger, rand_val))
			return region;
		}

	return nullptr;
}

bool SFZSound::has_region_for(int note, SFZRegion::Trigger trigger)
{
	int num_regions = regions.size();
	for (int i = 0; i < num_regions; ++i) {
		SFZRegion* region = regions[i];
		if (region->ever_matches(note, trigger))
			return true;
		}
	return false;
}

int SFZSound::group_for(int note, SFZRegion::Trigger trigger)
{
	int num_regions = regions.size();
	for (int i = 0; i < num_regions; ++i) {
		SFZRegion* region = regions[i];
		if (region->ever_matches(note, trigger))
			return region->group;
		}
	return 0;
}


std::string SFZSound::get_errors_string()
{
	std::string result;
	int num_errors = errors.size();
	for (int i = 0; i < num_errors; ++i)
		result += errors[i] + "\n";

	if (unsupported_opcodes.size() > 0) {
		result += "\nUnsupported opcodes:";
		bool shown_one = false;
		for (const auto& kv: unsupported_opcodes) {
			if (!shown_one) {
				result += " ";
				shown_one = true;
				}
			else
				result += ", ";
			result += kv.first;
			}
		result += "\n";
		}
	return result;
}


int SFZSound::num_subsounds()
{
	return 1;
}


std::string SFZSound::subsound_name(int which_subsound)
{
	return std::string();
}


void SFZSound::use_subsound(int which_subsound)
{
}


int SFZSound::selected_subsound()
{
	return 0;
}


void SFZSound::dump()
{
	int i;

	int num_errors = errors.size();
	if (num_errors > 0) {
		std::cout << "Errors:" << std::endl;
		for (i = 0; i < num_errors; ++i)
			std::cout << "- " << errors[i] << std::endl;
		std::cout << std::endl;
		}

	if (unsupported_opcodes.size() > 0) {
		std::cout << "Unused opcodes:" << std::endl;
		for (const auto& kv: unsupported_opcodes)
			std::cout << "  " << kv.first << std::endl;
		std::cout << std::endl;
		}

	std::cout << "Regions:" << std::endl;
	for (const auto region: regions)
		region->dump();
	std::cout << std::endl;

	std::cout << "Samples:" << std::endl;
	for (const auto& kv: samples)
		kv.second->dump();
}


std::string SFZSound::sibling_path(std::string from, std::string filename)
{
	// Leave absolute paths untouched.
	if (filename[0] == '/')
		return filename;
	return from.substr(0, from.find_last_of('/') + 1) + filename;
}

std::string SFZSound::fix_slashes(std::string str)
{
	std::ostringstream result;
	std::string::size_type start = 0;
	while (true) {
		auto slash_pos = str.find_first_of('\\', start);
		if (slash_pos == std::string::npos) {
			result << str.substr(start);
			break;
			}
		result << str.substr(start, slash_pos - start) << '/';
		start = slash_pos + 1;
		}
	return result.str();
}



