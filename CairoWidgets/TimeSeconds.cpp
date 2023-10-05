#include "TimeSeconds.h"


TimeSeconds TimeSeconds::operator+(const TimeSeconds& other) const
{
	TimeSeconds result = *this;
	result.time.tv_sec += other.time.tv_sec;
	result.time.tv_nsec += other.time.tv_nsec;
	if (result.time.tv_nsec > 1000000000) {
		result.time.tv_sec += 1;
		result.time.tv_nsec -= 1000000000;
		}
	return result;
}

TimeSeconds TimeSeconds::operator-(const TimeSeconds& other) const
{
	TimeSeconds result = *this;
	result.time.tv_sec -= other.time.tv_sec;
	result.time.tv_nsec -= other.time.tv_nsec;
	if (result.time.tv_nsec < 0) {
		result.time.tv_sec -= 1;
		result.time.tv_nsec += 1000000000;
		}
	return result;
}


double TimeSeconds::as_double() const
{
	return time.tv_sec + time.tv_nsec / 1000000000.0;
}



