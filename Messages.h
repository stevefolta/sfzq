#pragma once


enum {
	// Main thread -> audio thread.
	UseSound, 	// param: SFZSound*
		// Passes ownership of the SFZSound to the audio thread.
	UseSubsound, 	// num: which_subsound

	// Audio thread -> main thread.
	DoneWithSound, 	// param: SFZSound*
		// Passes ownership of SFZSound back to the main thread, so it can be deleted.
	SubsoundChanged,
	VoicesUsed, 	// num: number of voices used.

	// Sample load thread -> main thread.
	SampleLoadComplete,
	};
