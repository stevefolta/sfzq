#pragma once


struct SF2Generator {
	enum Type {
		Word, Short, Range
		};

	const char* name;
	Type	type;

	#define SF2GeneratorValue(name, type)	name
	enum {
		#include "sf2-chunks/generators.h"
		};
	#undef SF2GeneratorValue
	};

extern const SF2Generator* generator_for(unsigned short index);

