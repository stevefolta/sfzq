#pragma once

#include <math.h>


inline double note_hz(double note, const double freq_of_a = 440.0)
{
	note -= 12 * 5 + 9;
	// Now 0 = A
	return freq_of_a * pow(2.0, note / 12.0);
}


inline double note_for_frequency(double hz, const double freq_of_a = 440.0)
{
	auto note = 12.0 * log2(hz / freq_of_a);
	return note + 12 * 5 + 9;
}



