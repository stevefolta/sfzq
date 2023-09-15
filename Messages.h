#pragma once


enum {
	// Main thread -> audio thread.
	UseSound, 	// param: SFZSound*
		// Passes ownership of the SFZSound to the audio thread.

	// Audio thread -> main thread.
	DoneWithSound, 	// param: SFZSound*
		// Passes ownership of SFZSound back to the main thread, so it can be deleted.

	// Sample load thread -> main thread.
	SampleLoadComplete,
	};
