#pragma once


enum {
	// Main thread -> audio thread.
	UseSound, 	// param: SFZSound*
		// Passes ownership of the SFZSound to the audio thread.
	UseSubsound, 	// num: which_subsound
	UseTuning, 	// param: Tuning*

	// Audio thread -> main thread.
	DoneWithSound, 	// param: SFZSound*
		// Passes ownership of SFZSound back to the main thread, so it can be deleted.
	SubsoundChanged,
	VoicesUsed, 	// num: number of voices used.
	ActiveKeys0, 	// num: bitmap of active keys 0-63
	ActiveKeys1, 	// num: bitmap of active keys 64-127
	KeyDown, 	// num: key
	KeyUp, 	// num: key
	DoneWithTuning, 	// param: Tuning*
		// Passes ownership of Tuning back to the main thread, so it can be deleted.

	// Sample load thread -> main thread.
	SampleLoadComplete,
	};
