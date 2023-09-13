#include "SFZEG.h"
#include <math.h>


static const float fast_release_time = 0.01;


SFZEG::SFZEG()
	: exponential_decay(false)
{
}


void SFZEG::set_exponential_decay(bool new_exponential_decay)
{
	exponential_decay = new_exponential_decay;
}


void SFZEG::start_note(
	const SFZEGParameters* new_parameters, float float_velocity,
	float new_sample_rate,
	const SFZEGParameters* vel_mod)
{
	parameters = *new_parameters;
	if (vel_mod) {
		parameters.delay += float_velocity * vel_mod->delay;
		parameters.attack += float_velocity * vel_mod->attack;
		parameters.hold += float_velocity * vel_mod->hold;
		parameters.decay += float_velocity * vel_mod->decay;
		parameters.sustain += float_velocity * vel_mod->sustain;
		if (parameters.sustain < 0.0)
			parameters.sustain = 0.0;
		else if (parameters.sustain > 100.0)
			parameters.sustain = 100.0;
		parameters.release += float_velocity * vel_mod->release;
		}
	sample_rate = new_sample_rate;

	start_delay();
}


void SFZEG::next_segment()
{
	switch (segment) {
		case Delay:
			start_attack();
			break;

		case Attack:
			start_hold();
			break;

		case Hold:
			start_decay();
			break;

		case Decay:
			start_sustain();
			break;

		case Sustain:
			// Shouldn't be called.
			break;

		case Release:
		default:
			segment = Done;
			break;
		}
}


void SFZEG::note_off()
{
	start_release();
}


void SFZEG::fast_release()
{
	segment = Release;
	samples_until_next_segment = fast_release_time * sample_rate;
	slope = -level / samples_until_next_segment;
	segment_is_exponential = false;
}


void SFZEG::start_delay()
{
	if (parameters.delay <= 0)
		start_attack();
	else {
		segment = Delay;
		level = 0.0;
		slope = 0.0;
		samples_until_next_segment = parameters.delay * sample_rate;
		segment_is_exponential = false;
		}
}


void SFZEG::start_attack()
{
	if (parameters.attack <= 0)
		start_hold();
	else {
		segment = Attack;
		level = parameters.start / 100.0;
		samples_until_next_segment = parameters.attack * sample_rate;
		slope = 1.0 / samples_until_next_segment;
		segment_is_exponential = false;
		}
}


void SFZEG::start_hold()
{
	if (parameters.hold <= 0) {
		level = 1.0;
		start_decay();
		}
	else {
		segment = Hold;
		samples_until_next_segment = parameters.hold * sample_rate;
		level = 1.0;
		slope = 0.0;
		segment_is_exponential = false;
		}
}


void SFZEG::start_decay()
{
	if (parameters.decay <= 0)
		start_sustain();
	else {
		segment = Decay;
		samples_until_next_segment = parameters.decay * sample_rate;
		level = 1.0;
		if (exponential_decay) {
			// I don't truly understand this; just following what LinuxSampler does.
			float mystery_slope = -9.226 / samples_until_next_segment;
			slope = exp(mystery_slope);
			segment_is_exponential = true;
			if (parameters.sustain > 0.0) {
				// Again, this is following LinuxSampler's example, which is similar to
				// SF2-style decay, where "decay" specifies the time it would take to
				// get to zero, not to the sustain level.  The SFZ spec is not that
				// specific about what "decay" means, so perhaps it's really supposed
				// to specify the time to reach the sustain level.
				samples_until_next_segment =
					(long) (log((parameters.sustain / 100.0) / level) / mystery_slope);
				if (samples_until_next_segment <= 0)
					start_sustain();
				}
			}
		else {
			slope = (parameters.sustain / 100.0 - 1.0) / samples_until_next_segment;
			segment_is_exponential = false;
			}
		}
}


void SFZEG::start_sustain()
{
	if (parameters.sustain <= 0)
		start_release();
	else {
		segment = Sustain;
		level = parameters.sustain / 100.0;
		slope = 0.0;
		samples_until_next_segment = 0x7FFFFFFF;
		segment_is_exponential = false;
		}
}


void SFZEG::start_release()
{
	float release = parameters.release;
	if (release <= 0) {
		// Enforce a short release, to prevent clicks.
		release = fast_release_time;
		}

	segment = Release;
	samples_until_next_segment = release * sample_rate;
	if (exponential_decay) {
		// I don't truly understand this; just following what LinuxSampler does.
		float mystery_slope = -9.226 / samples_until_next_segment;
		slope = exp(mystery_slope);
		segment_is_exponential = true;
		}
	else {
		slope = -level / samples_until_next_segment;
		segment_is_exponential = false;
		}
}



