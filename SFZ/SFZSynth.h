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

		int num_voices_used();
		std::string voice_info_string();

	protected:
		std::vector<SFZVoice*> voices;
		SFZSound* sound = nullptr;
		unsigned char note_velocities[128];
	};

