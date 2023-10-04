#pragma once

class SFZSample;


// SFZRegion is designed to be able to be bitwise-copied.

struct SFZEGParameters {
	float	delay, start, attack, hold, decay, sustain, release;

	void	clear();
	void	clear_mod();
	};

class SFZRegion {
	public:
		enum Trigger {
			attack, release, first, legato
			};
		enum LoopMode {
			sample_loop, no_loop, one_shot, loop_continuous, loop_sustain
			};
		enum OffMode {
			fast, normal
			};


		SFZRegion();
		void clear();
		void clear_for_sf2();
		void clear_for_relative_sf2();
		void add_for_sf2(SFZRegion* other);
		void sf2_to_sfz();
		void dump();

		bool matches(unsigned char note, unsigned char velocity, Trigger trigger, float rand_val) {
			bool ok =
				note >= lokey && note <= hikey &&
				velocity >= lovel && velocity <= hivel &&
				(trigger == this->trigger ||
				 (this->trigger == attack && (trigger == first || trigger == legato))) &&
				rand_val >= lorand && rand_val < hirand;
			if (!ok)
				return false;
			ok = (cur_seq_position == seq_position);
			cur_seq_position += 1; 
			if (cur_seq_position > seq_length)
				cur_seq_position = 1;
			return ok;
			}
		bool ever_matches(unsigned char note, Trigger trigger) {
			return
				note >= lokey && note <= hikey &&
				(trigger == this->trigger ||
				 (this->trigger == attack && (trigger == first || trigger == legato)));
			}

		SFZSample* sample;
		unsigned char lokey, hikey;
		unsigned char lovel, hivel;
		Trigger trigger;
		float lorand = 0.0, hirand = 1.0;
		int seq_position = 1, seq_length = 1;
		unsigned long group, off_by;
		OffMode off_mode;

		unsigned long offset;
		unsigned long end;
		bool negative_end;
		LoopMode loop_mode;
		unsigned long loop_start, loop_end;
		int transpose;
		int tune;
		int pitch_keycenter, pitch_keytrack;
		int bend_up, bend_down;
		int cur_seq_position = 1;

		float volume, pan;
		float amp_veltrack;

		SFZEGParameters ampeg, ampeg_veltrack;

		static float timecents_to_secs(short timecents);
	};



