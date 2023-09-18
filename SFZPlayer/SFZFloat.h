#pragma once

// This is the type we'll do our calculations in (mainly in SFZVoice::render()).
// Normally, this will be a double, even if the host is operating at 32-bits.
// But if built with SUPPORT_32_BIT_ONLY, the calculations will be done as
// floats.

#ifdef SUPPORT_32_BIT_ONLY
	typedef float sfz_float;
#else
	typedef double sfz_float;
#endif

