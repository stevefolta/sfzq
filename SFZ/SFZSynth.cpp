#include "SFZSynth.h"
#include "SFZVoice.h"
#include "SFZSound.h"
#include <sstream>



SFZSynth::SFZSynth(int num_voices)
{
	while (num_voices-- > 0)
		voices.push_back(new SFZVoice());
}

SFZSynth::~SFZSynth()
{
	for (auto voice: voices)
		delete voice;
}


void SFZSynth::note_on(int channel, int note, double velocity)
{
	int midi_velocity = (int) (velocity * 127);

	// First, stop any currently-playing sounds in the group.
	int group = 0;
	if (sound) {
		SFZRegion* region = sound->get_region_for(note, midi_velocity);
		if (region)
			group = region->group;
		}
	if (group != 0) {
		for (auto voice: voices) {
			if (voice->get_off_by() == group)
				voice->stop_note_for_group();
			}
		}

	// Are any notes playing?  (Needed for first/legato trigger handling.)
	// Also stop any voices still playing this note.
	bool any_notes_playing = false;
	for (auto voice: voices) {
		if (voice->is_playing_note_down()) {
			if (voice->currently_playing_note() == note) {
				if (!voice->is_playing_one_shot())
					voice->stop_note_quick();
				}
			else
				any_notes_playing = true;
			}
		}

	// Play *all* matching regions.
	SFZRegion::Trigger trigger =
		(any_notes_playing ? SFZRegion::legato : SFZRegion::first);
	if (sound) {
		int num_regions = sound->num_regions();
		for (int i = 0; i < num_regions; ++i) {
			SFZRegion* region = sound->region_at(i);
			if (region->matches(note, midi_velocity, trigger)) {
				SFZVoice* voice = find_free_voice(note, is_note_stealing_enabled());
				if (voice) {
					voice->set_region(region);
					voice->start_note(note, velocity, sound, cur_tuning_expression);
					}
				}
			}
		}

	note_velocities[note] = midi_velocity;
}


void SFZSynth::note_off(
	int channel, int note,
	double velocity, bool allow_tail_off)
{
	// Start release region.
	if (sound) {
		SFZRegion* region =
			sound->get_region_for(
				note, note_velocities[note], SFZRegion::release);
		if (region) {
			SFZVoice* voice = find_free_voice(note, false);
			if (voice) {
				voice->set_region(region);
				voice->start_note(note, note_velocities[note] / 127.0, sound, cur_tuning_expression);
				}
			}
		}
}


void SFZSynth::tuning_expression_changed(double new_tuning_expression)
{
	cur_tuning_expression = new_tuning_expression;
	for (auto voice: voices)
		voice->tuning_expression_changed(new_tuning_expression);
}


int SFZSynth::num_voices_used()
{
	int num_used = 0;
	for (auto voice: voices) {
		if (voice->is_playing())
			num_used += 1;
		}
	return num_used;
}


std::string SFZSynth::voice_info_string()
{
	enum {
		max_shown_voices = 20,
		};

	std::vector<std::string> lines;
	int num_used = 0, num_shown = 0;
	for (auto voice: voices) {
		if (!voice->is_playing())
			continue;
		num_used += 1;
		if (num_shown >= max_shown_voices)
			continue;
		lines.push_back(voice->info_string());
		}
	std::ostringstream result;
	result << "voices used: " << num_used << std::endl;
	for (auto& line: lines)
		result << line << std::endl;
	return result.str();
}


SFZVoice* SFZSynth::find_free_voice(int note, bool can_steal)
{
	// Look for a truly free voice.
	for (auto voice: voices) {
		if (!voice->is_playing())
			return voice;
		}

	if (!can_steal)
		return nullptr;

	//*** TODO:  implement voice-stealing.
	return nullptr;
}



