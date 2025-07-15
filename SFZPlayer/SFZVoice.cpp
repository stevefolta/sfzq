#include "SFZVoice.h"
#include "SFZSound.h"
#include "SFZRegion.h"
#include "SFZSample.h"
#include "SFZSynth.h"
#include "SampleBuffer.h"
#include "OutBuffer.h"
#include "Decibels.h"
#include "SFZFloat.h"
#include "MIDINoteFrequency.h"
#include "Tunings.h"
#include <sstream>
#include <math.h>
#include <iostream>

static const float global_gain = -1.0;


SFZVoice::SFZVoice(SFZSynth* synth_in)
	: synth(synth_in), region(nullptr)
{
	ampeg.set_exponential_decay(true);
}


SFZVoice::~SFZVoice()
{
}


void SFZVoice::start_note(
	int note_in,
	float float_velocity,
	int channel,
	int note_id,
	SFZSound* sound,
	double current_tuning_expression)
{
	if (sound == nullptr) {
		kill_note();
		return;
		}

	int velocity = (int) (float_velocity * 127.0);
	cur_velocity = velocity;
	if (region == nullptr) {
		// This is not really used.
		region = sound->get_region_for(note_in, velocity, 0.0);
		}
	if (region == nullptr || region->sample == nullptr || region->sample->buffer == nullptr) {
		kill_note();
		return;
		}
	if (region->negative_end) {
		kill_note();
		return;
		}

	// Pitch.
	cur_note = note_in;
	cur_channel = channel;
	cur_note_id = note_id;
	cur_tuning_expression = current_tuning_expression;
	calc_pitch_ratio();

	// Gain.
	double note_gain_dB = global_gain + region->volume;
	// Thanks to <http:://www.drealm.info/sfz/plj-sfz.xhtml> for explaining the
	// velocity curve in a way that I could understand, although they mean
	// "log10" when they say "log".
	double velocity_gain_dB = -20.0 * log10((127.0 * 127.0) / (velocity * velocity));
	velocity_gain_dB *= region->amp_veltrack / 100.0;
	note_gain_dB += velocity_gain_dB;
	note_gain_left = note_gain_right = decibels_to_gain(note_gain_dB);
	// The SFZ spec is silent about the pan curve, but a 3dB pan law seems
	// common.  This sqrt() curve matches what Dimension LE does; Alchemy Free
	// seems closer to sin(adjusted_pan * pi/2).
	double adjusted_pan = (region->pan + 100.0) / 200.0;
	note_gain_left *= sqrt(1.0 - adjusted_pan);
	note_gain_right *= sqrt(adjusted_pan);
	ampeg.start_note(
		&region->ampeg, float_velocity, sample_rate, &region->ampeg_veltrack);

	// Offset/end.
	source_sample_position = region->offset;
	sample_end = region->sample->num_samples;
	if (region->end > 0 && region->end < sample_end)
		sample_end = region->end + 1;

	// Loop.
	loop_start = loop_end = 0;
	SFZRegion::LoopMode loop_mode = region->loop_mode;
	if (loop_mode == SFZRegion::sample_loop) {
		if (region->sample->loop_start < region->sample->loop_end)
			loop_mode = SFZRegion::loop_continuous;
		else
			loop_mode = SFZRegion::no_loop;
		}
	if (loop_mode != SFZRegion::no_loop && loop_mode != SFZRegion::one_shot) {
		if (region->loop_start < region->loop_end) {
			loop_start = region->loop_start;
			loop_end = region->loop_end;
			}
		else {
			loop_start = region->sample->loop_start;
			loop_end = region->sample->loop_end;
			}
		}
	num_loops = 0;
}


void SFZVoice::stop_note(float velocity, const bool allow_tail_off)
{
	if (!allow_tail_off || region == nullptr) {
		kill_note();
		return;
		}

	if (region->loop_mode != SFZRegion::one_shot)
		ampeg.note_off();
	if (region->loop_mode == SFZRegion::loop_sustain) {
		// Continue playing, but stop looping.
		loop_end = loop_start;
		}
}

void SFZVoice::stop_note_for_group()
{
	if (region->off_mode == SFZRegion::fast)
		ampeg.fast_release();
	else
		ampeg.note_off();
}

void SFZVoice::stop_note_quick()
{
	ampeg.fast_release();
}

void SFZVoice::kill_note()
{
	if (region)
		synth->note_ended(cur_note, cur_channel, cur_note_id);
	region = nullptr;
}


void SFZVoice::tuning_expression_changed(double new_value)
{
	if (region == nullptr)
		return;

	cur_tuning_expression = new_value;
	calc_pitch_ratio();
}


void SFZVoice::render(
	OutBuffer* output_buffer, int start_sample, int num_samples)
{
	if (region == nullptr)
		return;

	SampleBuffer* in_buffer = region->sample->buffer;
	auto in_l = in_buffer->channel_start(0);
	auto in_r = in_buffer->num_channels > 1 ? in_buffer->channel_start(1) : nullptr;
	auto in_read = in_buffer->read_sample;

#ifndef SUPPORT_32_BIT_ONLY
	double* out_l_64 = output_buffer->samples_for_channel_64(0);
	if (out_l_64)
		out_l_64 += start_sample;
	double* out_r_64 =
		output_buffer->num_channels() > 1 ?
		output_buffer->samples_for_channel_64(1) : nullptr;
	if (out_r_64)
		out_r_64 += start_sample;
#endif
	float* out_l_32 = output_buffer->samples_for_channel_32(0);
	if (out_l_32)
		out_l_32 += start_sample;
	float* out_r_32 =
		output_buffer->num_channels() > 1 ?
		output_buffer->samples_for_channel_32(1) : nullptr;
	if (out_r_32)
		out_r_32 += start_sample;

	// Cache some values, to give them at least some chance of ending up in
	// registers.
	double source_sample_position = this->source_sample_position;
	float ampeg_gain = ampeg.level;
	float ampeg_slope = ampeg.slope;
	long samples_until_next_amp_segment = ampeg.samples_until_next_segment;
	bool amp_segment_is_exponential = ampeg.segment_is_exponential;
	float loop_start = this->loop_start;
	float loop_end = this->loop_end;
	float sample_end = this->sample_end;

	while (--num_samples >= 0) {
		int pos = (int) source_sample_position;
		sfz_float alpha = (float) (source_sample_position - pos);
		sfz_float inv_alpha = 1.0f - alpha;
		int next_pos = pos + 1;
		if (loop_start < loop_end && next_pos > loop_end)
			next_pos = loop_start;

		// Simple linear interpolation.
		auto stride = in_buffer->stride;
		sfz_float l =
			in_read(in_l + pos * stride) * inv_alpha + in_read(in_l + next_pos * stride) * alpha;
		sfz_float r =
			in_r ?
			(in_read(in_r + pos * stride) * inv_alpha + in_read(in_r + next_pos * stride) * alpha) :
			l;

		sfz_float gain_left = note_gain_left * ampeg_gain;
		sfz_float gain_right = note_gain_right * ampeg_gain;
		l *= gain_left;
		r *= gain_right;
		// Shouldn't we dither here?

#ifndef SUPPORT_32_BIT_ONLY
		if (out_r_64) {
			*out_l_64++ += l;
			*out_r_64++ += r;
			}
		else if (out_l_64)
			*out_l_64++ += (l + r) * 0.5d;
		else
#endif
		if (out_r_32) {
			*out_l_32++ += l;
			*out_r_32++ += r;
			}
		else
			*out_l_32++ += (l + r) * 0.5f;

		// Next sample.
		source_sample_position += pitch_ratio;
		if (loop_start < loop_end && source_sample_position > loop_end) {
			source_sample_position = loop_start;
			num_loops += 1;
			}

		// Update EG.
		if (amp_segment_is_exponential)
			ampeg_gain *= ampeg_slope;
		else
			ampeg_gain += ampeg_slope;
		if (--samples_until_next_amp_segment < 0) {
			ampeg.level = ampeg_gain;
			ampeg.next_segment();
			ampeg_gain = ampeg.level;
			ampeg_slope = ampeg.slope;
			samples_until_next_amp_segment = ampeg.samples_until_next_segment;
			amp_segment_is_exponential = ampeg.segment_is_exponential;
			}

		if (source_sample_position >= sample_end || ampeg.is_done()) {
			kill_note();
			break;
			}
		}

	this->source_sample_position = source_sample_position;
	ampeg.level = ampeg_gain;
	ampeg.samples_until_next_segment = samples_until_next_amp_segment;
}


bool SFZVoice::is_playing_note_down()
{
	return (region && region->trigger != SFZRegion::release);
}


bool SFZVoice::is_playing_one_shot()
{
	return (region && region->loop_mode == SFZRegion::one_shot);
}


int SFZVoice::get_group()
{
	return (region ? region->group : 0);
}


int SFZVoice::get_off_by()
{
	return (region ? region->off_by : 0);
}


void SFZVoice::set_region(SFZRegion* next_region)
{
	region = next_region;
}


std::string SFZVoice::info_string()
{
	const char* eg_segment_names[] = {
		"delay", "attack", "hold", "decay", "sustain", "release", "done"
		};
	#define num_eg_segments (sizeof(eg_segment_names) / sizeof(eg_segment_names[0]))
	const char* eg_segment_name = "-Invalid-";
	int eg_segment_index = ampeg.segment_index();
	if (eg_segment_index >= 0 && eg_segment_index < (int) num_eg_segments)
		eg_segment_name = eg_segment_names[eg_segment_index];

	std::ostringstream result;
	result <<
		"note: " << cur_note << ", vel: " << cur_velocity <<
		", pan: " << region->pan << ", eg: " << eg_segment_name <<
		", loops: " << num_loops << std::endl;
	return result.str();
}



void SFZVoice::calc_pitch_ratio()
{
	double note = cur_note;
	note += region->transpose;
	double target_freq;

	if (synth->tuning) {
		if (region->pitch_keytrack == 0.0)
			target_freq = note_hz(region->pitch_keycenter);
		else {
			// Treat it like region->pitch_keytrack == 100.0, even if it's not.
			target_freq = synth->tuning->frequencyForMidiNote(note);
			}
		if (cur_tuning_expression != 0 || region->tune != 0) {
			// region->tune, region->bend_up, and region->bend_down are in semitones,
			// which is inherently 12-TET.  So we'll convert to a floating-point 12-TET
			// note, apply those, and convert back to frequency.
			double semitones = region->tune / 100.00;
			if (cur_tuning_expression > 0)
				semitones += (cur_tuning_expression / 120.0) * region->bend_up / 100.0;
			else
				semitones -= (cur_tuning_expression / 120.0) * region->bend_down / 100.0;
			target_freq = note_hz(note_for_frequency(target_freq) + semitones);
			}
		}

	else {
		note += region->tune / 100.0;

		double adjusted_pitch =
			region->pitch_keycenter +
			(note - region->pitch_keycenter) * (region->pitch_keytrack / 100.0);
		if (cur_tuning_expression != 0.0) {
			// CLAP's pitch expression is in the range -120.0 to 120.0 semitones.  But
			// SFZ also specifies a "bend_up" and "bend_down" range, defaulting to -200
			// and +200 cents, with a max of +/- 9600 cents.  Here, we'll assume that
			// CLAP will normally give us the full range of -120.0 to +120.0 semitones.
			if (cur_tuning_expression > 0)
				adjusted_pitch += (cur_tuning_expression / 120.0) * region->bend_up / 100.0;
			else
				adjusted_pitch -= (cur_tuning_expression / 120.0) * region->bend_down / 100.0;
			}
		target_freq = note_hz(adjusted_pitch);
		}

	double natural_freq = note_hz(region->pitch_keycenter);
	pitch_ratio =
		(target_freq * region->sample->sample_rate) /
		(natural_freq * sample_rate);
}



