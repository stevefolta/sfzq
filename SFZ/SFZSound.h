#pragma once

#include "SFZRegion.h"
#include <string>
#include <map>
#include <vector>

class SFZSample;


class SFZSound {
	public:
		SFZSound(std::string path);
		virtual ~SFZSound();

		void add_region(SFZRegion* region); 	// Takes ownership of the region.
		SFZSample* add_sample(std::string path, std::string default_path = "");
		void add_error(const std::string& message);
		void add_unsupported_opcode(const std::string& opcode);

		virtual void load_regions();
		virtual void load_samples(double* progress_var = NULL);

		SFZRegion* get_region_for(
			int note, int velocity, SFZRegion::Trigger trigger = SFZRegion::attack);
		int num_regions() { return regions.size(); }
		SFZRegion* region_at(int index) { return regions[index]; }

		std::string get_path() { return path; }
		std::string get_errors_string();

		virtual int num_subsounds();
		virtual std::string subsound_name(int which_subsound);
		virtual void use_subsound(int which_subsound);
		virtual int selected_subsound();

		void dump();

	protected:
		std::string path;
		std::vector<SFZRegion*> regions;
		std::map<std::string, SFZSample*> samples;
		std::vector<std::string> errors;
		std::map<std::string, std::string> unsupported_opcodes;

		std::string sibling_path(std::string from, std::string filename);
		std::string fix_slashes(std::string str);
	};


