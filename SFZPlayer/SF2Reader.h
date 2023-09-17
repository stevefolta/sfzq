#pragma once

#include "SF2.h"
#include <string>
#include <fstream>
#include <functional>

class SF2Sound;
class SFZRegion;
class SFZSample;
class SampleBuffer;


class SF2Reader {
	public:
		SF2Reader(SF2Sound* sound, std::string path);
		~SF2Reader();

		void read();
		SampleBuffer* read_samples(std::function<void(double)> progress_fn = {});

	protected:
		SF2Sound* sound;
		std::fstream* file;

		void add_generator_to_region(
			word genOper, SF2::genAmountType* amount, SFZRegion* region);
	};

