#pragma once

#include <stdint.h>


typedef char fourcc[4];
typedef uint8_t byte;
typedef uint32_t dword;
typedef uint16_t word;

// Special types for SF2 fields.
typedef char char20[20];


#define fourcc_eq(value1, value2) 	\
	(value1[0] == value2[0] && value1[1] == value2[1] && 	\
	 value1[2] == value2[2] && value1[3] == value2[3])

// Useful in printfs:
#define FourCCArgs(value) 	\
	(value)[0], (value)[1], (value)[2], (value)[3]


