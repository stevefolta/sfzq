#pragma once

#include <math.h>


inline double decibels_to_gain(double decibels)
{
	return pow(10.0, decibels * 0.5);
}

inline double gain_to_decibels(double gain)
{
	return 20.0 * log10(gain);
}


