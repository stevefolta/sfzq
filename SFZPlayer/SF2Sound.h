#pragma once

#include "SFZSound.h"
#include <string>
#include <vector>
#include <map>

class SampleBuffer;


class SF2Sound : public SFZSound {
	public:
		SF2Sound(std::string path);
		~SF2Sound();

		void load_regions();
		void load_samples(std::function<void(double)> progress_fn = {});

		struct Preset {
			std::string name;
			int bank;
			int preset;
			std::vector<SFZRegion*> regions;

			Preset(std::string name_in, int bank_in, int preset_in)
				: name(name_in), bank(bank_in), preset(preset_in) {}
			~Preset() {}

			void add_region(SFZRegion* region) {
				regions.push_back(region);
				}
			};
		void add_preset(Preset* preset);

		int num_subsounds();
		std::string subsound_name(int which_subsound);
		void use_subsound(int which_subsound);
		int selected_subsound();

		SFZSample* sample_for(unsigned long sample_rate);
		void set_samples_buffer(SampleBuffer* buffer);

	protected:
		std::vector<Preset*> presets;
		std::map<int64_t, SFZSample*> samples_by_rate;
		int selected_preset;
	};

