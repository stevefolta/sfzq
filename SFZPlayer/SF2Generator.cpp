#include "SF2Generator.h"
#include <vector>


#define SF2GeneratorValue(name, type) 	\
	{ #name, SF2Generator::type }
static const std::vector<SF2Generator> generators = {
	#include "sf2-chunks/generators.h"
	};
#undef SF2GeneratorValue


const SF2Generator* generator_for(unsigned short index)
{
	if (index >= generators.size())
		return nullptr;
	return &generators[index];
}



