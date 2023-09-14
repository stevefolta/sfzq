#pragma once

#include <vector>
#include <string>

class SFZVoice;
class SFZSound;


class SFZSynth {
	public:
		SFZSynth(int num_voices);
		~SFZSynth();

		void set_sound(SFZSound* new_sound) { sound = new_sound; }

		void note_on(int midi_channel, int note, double velocity);
		void note_off(
			int midi_channel, int note,
			double velocity, bool allow_tail_off);
		void tuning_expression_changed(double new_tuning_expression);

		int num_voices_used();
		std::string voice_info_string();

		bool is_note_stealing_enabled() { return true; }

	protected:
		std::vector<SFZVoice*> voices;
		SFZSound* sound = nullptr;
		unsigned char note_velocities[128];
		double cur_tuning_expression = 0.0;
			// Basically, the position of the pitch wheel, as a CLAP tuning
			// expression value in semitones, with the range -120.0 to 120.0.


		SFZVoice* find_free_voice(int note, bool can_steal);
	};

