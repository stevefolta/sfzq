#pragma once

#include "SFZRegion.h"


class SFZEG {
	public:
		SFZEG();

		void set_exponential_decay(bool new_exponential_decay);
		void start_note(
			const SFZEGParameters* parameters, float float_velocity, float sample_rate,
			const SFZEGParameters* vel_mod = 0);
		void next_segment();
		void note_off();
		void fast_release();
		bool is_done() { return segment == Done; }
		bool is_releasing() { return segment == Release; }
		int segment_index() { return (int) segment; }

		float level;
		float slope;
		long samples_until_next_segment;
		bool segment_is_exponential;

	protected:
		enum Segment {
			Delay, Attack, Hold, Decay, Sustain, Release, Done
			};
		Segment segment;
		SFZEGParameters parameters;
		float sample_rate;
		bool exponential_decay;

		void start_delay();
		void start_attack();
		void start_hold();
		void start_decay();
		void start_sustain();
		void start_release();
	};



