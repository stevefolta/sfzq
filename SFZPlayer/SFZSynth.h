#pragma once

#include <vector>
#include <string>
#include <functional>

class SFZVoice;
class SFZSound;
class OutBuffer;
namespace Tunings {
	class Tuning;
	}


class SFZSynth {
	public:
		SFZSynth(int num_voices);
		~SFZSynth();

		SFZSound* set_sound(SFZSound* new_sound) {
			auto old_sound = sound;
			sound = new_sound;
			return old_sound;
			}
		void set_note_off_fn(std::function<void(int note, int channel, int note_id)> new_fn) { note_off_fn = new_fn; }
		void set_sample_rate(double new_sample_rate);
		void use_subsound(int which_subsound);
		void reset();
		bool note_is_active(int note);

		void note_on(int note, double velocity, int channel, int note_id);
		void note_off(int note, double velocity, int channel, int note_id, bool allow_tail_off);
		void tuning_expression_changed(double new_tuning_expression);
		void render(
			OutBuffer* output_buffer, int start_sample, int num_samples);

		int num_voices_used();
		std::string voice_info_string();
		int selected_subsound();

		bool is_note_stealing_enabled() { return true; }

		// Called by SFZVoices.
		void note_ended(int note, int channel, int note_id);
		double get_tuning_expression() { return cur_tuning_expression; }

		Tunings::Tuning* tuning = nullptr;

	protected:
		std::vector<SFZVoice*> voices;
		SFZSound* sound = nullptr;
		unsigned char note_velocities[128];
		double cur_tuning_expression = 0.0;
			// Basically, the position of the pitch wheel, as a CLAP tuning
			// expression value in semitones, with the range -120.0 to 120.0.
		std::function<void(int note, int channel, int note_id)> note_off_fn;

		SFZVoice* find_free_voice(int note, bool can_steal);
	};

