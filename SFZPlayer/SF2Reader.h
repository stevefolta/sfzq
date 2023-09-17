#pragma once

#include "SF2.h"
#include <string>
#include <fstream>


class SF2Sound;
class SFZRegion;
class SFZSample;


class SF2Reader {
	public:
		SF2Reader(SF2Sound* sound, std::string path);
		~SF2Reader();

		void	read();
		SampleBuffer*	read_samples(double* progress_var = NULL);

	protected:
		SF2Sound*	sound;
		std::fstream*	file;

		void	add_generator_to_region(
			word genOper, SF2::genAmountType* amount, SFZRegion* region);
	};

