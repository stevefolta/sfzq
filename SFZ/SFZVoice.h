#pragma once

#include "SFZEG.h"
#include <string>

class SFZSound;
class SFZRegion;
class OutBuffer;


class SFZVoice {
	public:
		SFZVoice();
		~SFZVoice();

		void start_note(
			const int note,
			const float velocity,
			SFZSound* sound,
			double current_tuning_expression);
		void stop_note(float velocity, const bool allow_tail_off);
		void stop_note_for_group();
		void stop_note_quick();
		void kill_note();
		void tuning_expression_changed(double new_value);
		void render_next_block(
			OutBuffer* output_buffer, int start_sample, int num_samples);
		bool is_playing_note_down();
		bool is_playing_one_shot();
		bool is_playing() { return region != nullptr; }
		int currently_playing_note() { return cur_note; }

		int get_group();
		int get_off_by();

		// Set the region to be used by the next start_note().
		void set_region(SFZRegion* next_region);

		std::string info_string();

		double sample_rate = 0.0;

	protected:
		int trigger;
		SFZRegion* region;
		int cur_note;
		double cur_tuning_expression;
		double pitch_ratio;
		float note_gain_left, note_gain_right;
		double source_sample_position;
		SFZEG ampeg;
		unsigned long sample_end;
		unsigned long loop_start, loop_end;

		// Info only.
		unsigned long num_loops;
		int cur_velocity;

		void calc_pitch_ratio();
		void note_ended();
		double note_hz(double note, const double freq_of_a = 440.0);
	};


