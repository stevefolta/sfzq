#include "SF2Reader.h"
#include "SF2Sound.h"
#include "SFZSample.h"
#include "SampleBuffer.h"
#include "RIFF.h"
#include "SF2.h"
#include "SF2Generator.h"
#include <iostream>


SF2Reader::SF2Reader(SF2Sound* sound_in, std::string path)
	: sound(sound_in)
{
	file = new std::fstream(path);
}


SF2Reader::~SF2Reader()
{
	delete file;
}


void SF2Reader::read()
{
	if (file == nullptr) {
		sound->add_error("Couldn't open file.");
		return;
		}

	// Read the hydra.
	SF2::Hydra hydra;
	file->seekg(0);
	RIFFChunk riff_chunk;
	riff_chunk.read_from(file);
	while (file->tellg() < riff_chunk.end()) {
		RIFFChunk chunk;
		chunk.read_from(file);
		if (fourcc_eq(chunk.id, "pdta")) {
			hydra.read_from(file, chunk.end());
			break;
			}
		chunk.seek_after(file);
		}
	if (!hydra.is_complete()) {
		sound->add_error("Invalid SF2 file (missing or incomplete hydra).");
		return;
		}

	// Read each preset.
	for (int which_preset = 0; which_preset < hydra.phdr_num_items - 1; ++which_preset) {
		SF2::phdr* phdr = &hydra.phdr_items[which_preset];
		SF2Sound::Preset* preset =
			new SF2Sound::Preset(phdr->presetName, phdr->bank, phdr->preset);
		sound->add_preset(preset);

		// Zones.
		//*** TODO: Handle global zone (modulators only).
		int zone_end = phdr[1].presetBagNdx;
		for (int which_zone = phdr->presetBagNdx; which_zone < zone_end; ++which_zone) {
			SF2::pbag* pbag = &hydra.pbag_items[which_zone];
			SFZRegion preset_region;
			preset_region.clear_for_relative_sf2();

			// Generators.
			int gen_end = pbag[1].genNdx;
			for (int which_gen = pbag->genNdx; which_gen < gen_end; ++which_gen) {
				SF2::pgen* pgen = &hydra.pgen_items[which_gen];

				// Instrument.
				if (pgen->genOper == SF2Generator::instrument) {
					word which_inst = pgen->genAmount.wordAmount;
					if (which_inst < hydra.inst_num_items) {
						SFZRegion inst_region;
						inst_region.clear_for_sf2();
						// Preset generators are supposed to be "relative" modifications of
						// the instrument settings, but that makes no sense for ranges.
						// For those, we'll have the instrument's generator take
						// precedence, though that may not be correct.
						inst_region.lokey = preset_region.lokey;
						inst_region.hikey = preset_region.hikey;
						inst_region.lovel = preset_region.lovel;
						inst_region.hivel = preset_region.hivel;

						SF2::inst* inst = &hydra.inst_items[which_inst];
						int first_zone = inst->instBagNdx;
						int zone_end = inst[1].instBagNdx;
						for (int which_zone = first_zone; which_zone < zone_end; ++which_zone) {
							SF2::ibag* ibag = &hydra.ibag_items[which_zone];

							// Generators.
							SFZRegion zone_region = inst_region;
							bool had_sample_id = false;
							int gen_end = ibag[1].instGenNdx;
							for (int which_gen = ibag->instGenNdx; which_gen < gen_end; ++which_gen) {
								SF2::igen* igen = &hydra.igen_items[which_gen];
								if (igen->genOper == SF2Generator::sampleID) {
									int which_sample = igen->genAmount.wordAmount;
									SF2::shdr* shdr = &hydra.shdr_items[which_sample];
									zone_region.add_for_sf2(&preset_region);
									zone_region.sf2_to_sfz();
									zone_region.offset += shdr->start;
									zone_region.end += shdr->end;
									zone_region.loop_start += shdr->startLoop;
									zone_region.loop_end += shdr->endLoop;
									if (shdr->endLoop > 0)
										zone_region.loop_end -= 1;
									if (zone_region.pitch_keycenter == -1)
										zone_region.pitch_keycenter = shdr->originalPitch;
									zone_region.tune += shdr->pitchCorrection;

									// Pin initialAttenuation to max +6dB.
									if (zone_region.volume > 6.0) {
										zone_region.volume = 6.0;
										sound->add_unsupported_opcode("extreme gain in initialAttenuation");
										}

									SFZRegion* new_region = new SFZRegion();
									*new_region = zone_region;
									new_region->sample = sound->sample_for(shdr->sampleRate);
									preset->add_region(new_region);
									had_sample_id = true;
									}
								else
									add_generator_to_region(igen->genOper, &igen->genAmount, &zone_region);
								}

							// Handle instrument's global zone.
							if (which_zone == first_zone && !had_sample_id)
								inst_region = zone_region;

							// Modulators.
							int mod_end = ibag[1].instModNdx;
							int which_mod = ibag->instModNdx;
							if (which_mod < mod_end)
								sound->add_unsupported_opcode("any modulator");
							}
						}
					else
						sound->add_error("Instrument out of range.");
					}

				// Other generators.
				else
					add_generator_to_region(pgen->genOper, &pgen->genAmount, &preset_region);
				}

			// Modulators.
			int mod_end = pbag[1].modNdx;
			int which_mod = pbag->modNdx;
			if (which_mod < mod_end)
				sound->add_unsupported_opcode("any modulator");
			}
		}
}


SampleBuffer* SF2Reader::read_samples(std::function<void(double)> progress_fn)
{
	static const unsigned long buffer_size = 32768;

	if (file == nullptr) {
		sound->add_error("Couldn't open file.");
		return nullptr;
		}

	// Find the "sdta" chunk.
	file->seekg(0);
	RIFFChunk riff_chunk;
	riff_chunk.read_from(file);
	bool found = false;
	RIFFChunk chunk;
	while (file->tellg() < riff_chunk.end()) {
		chunk.read_from(file);
		if (fourcc_eq(chunk.id, "sdta")) {
			found = true;
			break;
			}
		chunk.seek_after(file);
		}
	int64_t sdta_end = chunk.end();
	found = false;
	while (file->tellg() < sdta_end) {
		chunk.read_from(file);
		if (fourcc_eq(chunk.id, "smpl")) {
			found = true;
			break;
			}
		chunk.seek_after(file);
		}
	if (!found) {
		sound->add_error("SF2 is missing its \"smpl\" chunk.");
		return nullptr;
		}

	// Allocate the SampleBuffer.
	unsigned long num_samples = chunk.size / sizeof(short);
	SampleBuffer* sample_buffer =
		new SampleBuffer(1, num_samples, 16, SampleBuffer::Little, SampleBuffer::Interleaved);

	// Read and convert.
	short* buffer = new short[buffer_size];
	unsigned long samples_left = num_samples;
	uint8_t* out = sample_buffer->channel_start(0);
	while (samples_left > 0) {
		// Read the buffer.
		unsigned long samples_to_read = buffer_size;
		if (samples_to_read > samples_left)
			samples_to_read = samples_left;
		file->read((char*) out, samples_to_read * sizeof(int16_t));

		samples_left -= samples_to_read;
		out += samples_to_read * sizeof(int16_t);

		if (progress_fn)
			progress_fn((double) (num_samples - samples_left) / num_samples);
		}
	delete[] buffer;

	if (progress_fn)
		progress_fn(1.0);

	return sample_buffer;
}


void SF2Reader::add_generator_to_region(
	word genOper, SF2::genAmountType* amount, SFZRegion* region)
{
	switch (genOper) {
		case SF2Generator::startAddrsOffset:
			region->offset += amount->shortAmount;
			break;
		case SF2Generator::endAddrsOffset:
			region->end += amount->shortAmount;
			break;
		case SF2Generator::startloopAddrsOffset:
			region->loop_start += amount->shortAmount;
			break;
		case SF2Generator::endloopAddrsOffset:
			region->loop_end += amount->shortAmount;
			break;
		case SF2Generator::startAddrsCoarseOffset:
			region->offset += amount->shortAmount * 32768;
			break;
		case SF2Generator::endAddrsCoarseOffset:
			region->end += amount->shortAmount * 32768;
			break;
		case SF2Generator::pan:
			region->pan = amount->shortAmount * (2.0 / 10.0);
			break;
		case SF2Generator::delayVolEnv:
			region->ampeg.delay = amount->shortAmount;
			break;
		case SF2Generator::attackVolEnv:
			region->ampeg.attack = amount->shortAmount;
			break;
		case SF2Generator::holdVolEnv:
			region->ampeg.hold = amount->shortAmount;
			break;
		case SF2Generator::decayVolEnv:
			region->ampeg.decay = amount->shortAmount;
			break;
		case SF2Generator::sustainVolEnv:
			region->ampeg.sustain = amount->shortAmount;
			break;
		case SF2Generator::releaseVolEnv:
			region->ampeg.release = amount->shortAmount;
			break;
		case SF2Generator::keyRange:
			region->lokey = amount->range.lo;
			region->hikey = amount->range.hi;
			break;
		case SF2Generator::velRange:
			region->lovel = amount->range.lo;
			region->hivel = amount->range.hi;
			break;
		case SF2Generator::startloopAddrsCoarseOffset:
			region->loop_start += amount->shortAmount * 32768;
			break;
		case SF2Generator::initialAttenuation:
			// The spec says "initialAttenuation" is in centibels.  But everyone
			// seems to treat it as millibels.
			region->volume += -amount->shortAmount / 100.0;
			break;
		case SF2Generator::endloopAddrsCoarseOffset:
			region->loop_end += amount->shortAmount * 32768;
			break;
		case SF2Generator::coarseTune:
			region->transpose += amount->shortAmount;
			break;
		case SF2Generator::fineTune:
			region->tune += amount->shortAmount;
			break;
		case SF2Generator::sampleModes:
			{
				SFZRegion::LoopMode loop_modes[] = {
					SFZRegion::no_loop, SFZRegion::loop_continuous,
					SFZRegion::no_loop, SFZRegion::loop_sustain };
				region->loop_mode = loop_modes[amount->wordAmount & 0x03];
			}
			break;
		case SF2Generator::scaleTuning:
			region->pitch_keytrack = amount->shortAmount;
			break;
		case SF2Generator::exclusiveClass:
			region->group = region->off_by = amount->wordAmount;
			break;
		case SF2Generator::overridingRootKey:
			region->pitch_keycenter = amount->shortAmount;
			break;
		case SF2Generator::endOper:
			// Ignore.
			break;

		case SF2Generator::modLfoToPitch:
		case SF2Generator::vibLfoToPitch:
		case SF2Generator::modEnvToPitch:
		case SF2Generator::initialFilterFc:
		case SF2Generator::initialFilterQ:
		case SF2Generator::modLfoToFilterFc:
		case SF2Generator::modEnvToFilterFc:
		case SF2Generator::modLfoToVolume:
		case SF2Generator::unused1:
		case SF2Generator::chorusEffectsSend:
		case SF2Generator::reverbEffectsSend:
		case SF2Generator::unused2:
		case SF2Generator::unused3:
		case SF2Generator::unused4:
		case SF2Generator::delayModLFO:
		case SF2Generator::freqModLFO:
		case SF2Generator::delayVibLFO:
		case SF2Generator::freqVibLFO:
		case SF2Generator::delayModEnv:
		case SF2Generator::attackModEnv:
		case SF2Generator::holdModEnv:
		case SF2Generator::decayModEnv:
		case SF2Generator::sustainModEnv:
		case SF2Generator::releaseModEnv:
		case SF2Generator::keynumToModEnvHold:
		case SF2Generator::keynumToModEnvDecay:
		case SF2Generator::keynumToVolEnvHold:
		case SF2Generator::keynumToVolEnvDecay:
		case SF2Generator::instrument:
			// Only allowed in certain places, where we already special-case it.
		case SF2Generator::reserved1:
		case SF2Generator::keynum:
		case SF2Generator::velocity:
		case SF2Generator::reserved2:
		case SF2Generator::sampleID:
			// Only allowed in certain places, where we already special-case it.
		case SF2Generator::reserved3:
		case SF2Generator::unused5:
			{
				const SF2Generator* generator = generator_for(genOper);
				sound->add_unsupported_opcode(generator->name);
			}
			break;
		}
}



