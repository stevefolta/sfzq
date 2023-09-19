#include "SFZReader.h"
#include "SFZRegion.h"
#include "SFZSound.h"
#include <fstream>
#include <sstream>
#include <string_view>


SFZReader::SFZReader(SFZSound* sound_in)
	: sound(sound_in), line(1)
{
}


SFZReader::~SFZReader()
{
}


void SFZReader::read(std::string path)
{
	// Read the entire file as a string.
	std::string contents;
	bool ok = false;
	if (std::ifstream file{path, std::ios::binary | std::ios::ate}) {
		auto size = file.tellg();
		contents = std::string(size, '\0');
		file.seekg(0);
		if (file.read(contents.data(), size))
			ok = true;
		}
	if (!ok) {
		sound->add_error("Couldn't read \"" + path + "\"");
		return;
		}

	read(contents.data(), contents.size());
}


void SFZReader::read(const char* text, unsigned int length)
{
	const char* p = text;
	const char* end = text + length;
	char c;

	SFZRegion cur_group;
	SFZRegion cur_region;
	SFZRegion* building_region = nullptr;
	bool in_control = false;
	std::string default_path;

	while (p < end) {
		// We're at the start of a line; skip any whitespace.
		while (p < end) {
			c = *p;
			if (c != ' ' && c != '\t')
				break;
			p += 1;
			}
		if (p >= end)
			break;

		// Check if it's a comment line.
		if (c == '/') {
			// Skip to end of line.
			while (p < end) {
				c = *++p;
				if (c == '\n' || c == '\r')
					break;
				}
			p = handle_line_end(p);
			continue;
			}

		// Check if it's a blank line.
		if (c == '\r' || c == '\n') {
			p = handle_line_end(p);
			continue;
			}

		// Handle elements on the line.
		while (p < end) {
			c = *p;

			// Tag.
			if (c == '<') {
				p += 1;
				const char* tag_start = p;
				while (p < end) {
					c = *p++;
					if (c == '\n' || c == '\r') {
						error("Unterminated tag");
						goto fatal_error;
						}
					else if (c == '>')
						break;
					}
				if (p >= end) {
					error("Unterminated tag");
					goto fatal_error;
					}
				std::string_view tag(tag_start, p - tag_start - 1);
				if (tag == "region") {
					if (building_region && building_region == &cur_region)
						finish_region(&cur_region);
					cur_region = cur_group;
					building_region = &cur_region;
					in_control = false;
					}
				else if (tag == "group") {
					if (building_region && building_region == &cur_region)
						finish_region(&cur_region);
					cur_group.clear();
					building_region = &cur_group;
					in_control = false;
					}
				else if (tag == "control") {
					if (building_region && building_region == &cur_region)
						finish_region(&cur_region);
					cur_group.clear();
					building_region = nullptr;
					in_control = true;
					}
				else
					error("Illegal tag");
				}

			// Comment.
			else if (c == '/') {
				// Skip to end of line.
				while (p < end) {
					c = *p;
					if (c == '\r' || c == '\n')
						break;
					p += 1;
					}
				}

			// Parameter.
			else {
				// Get the parameter name.
				const char* parameter_start = p;
				while (p < end) {
					c = *p++;
					if (c == '=' || c == ' ' || c == '\t' || c == '\r' || c == '\n')
						break;
					}
				if (p >= end || c != '=') {
					error("Malformed parameter");
					goto next_element;
					}
				std::string_view opcode(parameter_start, p - parameter_start - 1);
				if (in_control) {
					if (opcode == "default_path")
						p = read_path_into(&default_path, p, end);
					else {
						const char* value_start = p;
						while (p < end) {
							c = *p;
							if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
								break;
							p++;
							}
						std::string_view value(value_start, p - value_start);
						std::string faux_opcode =
							std::string(opcode.data(), opcode.size()) + " (in <control>)";
						sound->add_unsupported_opcode(faux_opcode);
						}
					}
				else if (opcode == "sample") {
					std::string path;
					p = read_path_into(&path, p, end);
					if (!path.empty()) {
						if (building_region)
							building_region->sample = sound->add_sample(path, default_path);
						else
							error("Adding sample outside a group or region");
						}
					else
						error("Empty sample path");
					}
				else {
					const char* value_start = p;
					while (p < end) {
						c = *p;
						if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
							break;
						p++;
						}
					std::string value(value_start, p - value_start);
					if (building_region == nullptr)
						error("Setting a parameter outside a region or group");
					else if (opcode == "lokey")
						building_region->lokey = key_value(value);
					else if (opcode == "hikey")
						building_region->hikey = key_value(value);
					else if (opcode == "key") {
						building_region->hikey =
						building_region->lokey =
						building_region->pitch_keycenter =
							key_value(value);
						}
					else if (opcode == "lovel")
						building_region->lovel = std::stol(value);
					else if (opcode == "hivel")
						building_region->hivel = std::stol(value);
					else if (opcode == "trigger")
						building_region->trigger = (SFZRegion::Trigger) trigger_value(value);
					else if (opcode == "group")
						building_region->group = (unsigned long) std::stol(value);
					else if (opcode == "off_by")
						building_region->off_by = (unsigned long) std::stol(value);
					else if (opcode == "offset")
						building_region->offset = (unsigned long) std::stol(value);
					else if (opcode == "end") {
						int64_t end = (unsigned long long) std::stoll(value);
						if (end < 0)
							building_region->negative_end = true;
						else
							building_region->end = end;
						}
					else if (opcode == "loop_mode") {
						bool models_supported =
							value == "no_loop" ||
							value == "one_shot" ||
							value == "loop_continuous";
						if (models_supported)
							building_region->loop_mode = (SFZRegion::LoopMode) loop_mode_value(value);
						else {
							std::string faux_opcode =
								std::string(opcode.data(), opcode.length()) + "=" + value;
							sound->add_unsupported_opcode(faux_opcode);
							}
						}
					else if (opcode == "loop_start")
						building_region->loop_start = (unsigned long) std::stol(value);
					else if (opcode == "loop_end")
						building_region->loop_end = (unsigned long) std::stol(value);
					else if (opcode == "transpose")
						building_region->transpose = std::stol(value);
					else if (opcode == "tune")
						building_region->tune = std::stol(value);
					else if (opcode == "pitch_keycenter")
						building_region->pitch_keycenter = key_value(value);
					else if (opcode == "pitch_keytrack")
						building_region->pitch_keytrack = std::stol(value);
					else if (opcode == "bend_up")
						building_region->bend_up = std::stol(value);
					else if (opcode == "bend_down")
						building_region->bend_down = std::stol(value);
					else if (opcode == "volume")
						building_region->volume = std::stof(value);
					else if (opcode == "pan")
						building_region->pan = std::stof(value);
					else if (opcode == "amp_veltrack")
						building_region->amp_veltrack = std::stof(value);
					else if (opcode == "ampeg_delay")
						building_region->ampeg.delay = std::stof(value);
					else if (opcode == "ampeg_start")
						building_region->ampeg.start = std::stof(value);
					else if (opcode == "ampeg_attack")
						building_region->ampeg.attack = std::stof(value);
					else if (opcode == "ampeg_hold")
						building_region->ampeg.hold = std::stof(value);
					else if (opcode == "ampeg_decay")
						building_region->ampeg.decay = std::stof(value);
					else if (opcode == "ampeg_sustain")
						building_region->ampeg.sustain = std::stof(value);
					else if (opcode == "ampeg_release")
						building_region->ampeg.release = std::stof(value);
					else if (opcode == "ampeg_vel2delay")
						building_region->ampeg_veltrack.delay = std::stof(value);
					else if (opcode == "ampeg_vel2attack")
						building_region->ampeg_veltrack.attack = std::stof(value);
					else if (opcode == "ampeg_vel2hold")
						building_region->ampeg_veltrack.hold = std::stof(value);
					else if (opcode == "ampeg_vel2decay")
						building_region->ampeg_veltrack.decay = std::stof(value);
					else if (opcode == "ampeg_vel2sustain")
						building_region->ampeg_veltrack.sustain = std::stof(value);
					else if (opcode == "ampeg_vel2release")
						building_region->ampeg_veltrack.release = std::stof(value);
					else if (opcode == "lorand")
						building_region->lorand = std::stof(value);
					else if (opcode == "hirand")
						building_region->hirand = std::stof(value);
					else if (opcode == "default_path")
						error("\"default_path\" outside of <control> tag");
					else
						sound->add_unsupported_opcode(std::string(opcode.data(), opcode.length()));
					}
				}

			// Skip to next element.
next_element:
			c = 0;
			while (p < end) {
				c = *p;
				if (c != ' ' && c != '\t')
					break;
				p += 1;
				}
			if (c == '\r' || c == '\n') {
				p = handle_line_end(p);
				break;
				}
			}
		}

fatal_error:
	if (building_region && building_region == &cur_region)
		finish_region(building_region);
}


const char* SFZReader::handle_line_end(const char* p)
{
	// Check for DOS-style line ending.
	char line_end_char = *p++;
	if (line_end_char == '\r' && *p == '\n')
		p += 1;
	line += 1;
	return p;
}


const char* SFZReader::read_path_into(
	std::string* path_out, const char* p_in, const char* end_in)
{
	// Paths are kind of funny to parse because they can contain whitespace.
	const char* p = p_in;
	const char* end = end_in;
	const char* path_start = p;
	const char* potential_end = nullptr;
	while (p < end) {
		char c = *p;
		if (c == ' ') {
			// Is this space part of the path?  Or the start of the next opcode?  We
			// don't know yet.
			potential_end = p;
			p += 1;
			// Skip any more spaces.
			while (p < end && *p == ' ')
				p += 1;
			}
		else if (c == '\n' || c == '\r' || c == '\t')
			break;
		else if (c == '=') {
			// We've been looking at an opcode; we need to rewind to
			// potential_end.
			p = potential_end;
			break;
			}
		p += 1;
		}
	if (p > path_start)
		path_out->assign(path_start, p - path_start);
	else
		path_out->clear();
	return p;
}


int SFZReader::key_value(const std::string& str)
{
	char c = str[0];
	if (c >= '0' && c <= '9')
		return std::stoi(str);

	int note = 0;
	static const int notes[] = {
		12 + 0, 12 + 2, 3, 5, 7, 8, 10,
		};
	if (c >= 'A' && c <= 'G')
		note = notes[c - 'A'];
	else if (c >= 'a' && c <= 'g')
		note = notes[c - 'a'];
	int octave_start = 1;
	c = str[1];
	if (c == 'b' || c == '#') {
		octave_start += 1;
		if (c == 'b')
			note -= 1;
		else
			note += 1;
		}
	int octave = std::stoi(str.substr(octave_start));
	// A3 == 57.
	int result = octave * 12 + note + (57 - 4 * 12);
	return result;
}


int SFZReader::trigger_value(const std::string& str)
{
	if (str == "release")
		return SFZRegion::release;
	else if (str == "first")
		return SFZRegion::first;
	else if (str == "legato")
		return SFZRegion::legato;
	return SFZRegion::attack;
}


int SFZReader::loop_mode_value(const std::string& str)
{
	if (str == "no_loop")
		return SFZRegion::no_loop;
	else if (str == "one_shot")
		return SFZRegion::one_shot;
	else if (str == "loop_continuous")
		return SFZRegion::loop_continuous;
	else if (str == "loop_sustain")
		return SFZRegion::loop_sustain;
	return SFZRegion::sample_loop;
}


void SFZReader::finish_region(SFZRegion* region)
{
	SFZRegion* new_region = new SFZRegion();
	*new_region = *region;
	sound->add_region(new_region);
}


void SFZReader::error(const std::string& message)
{
	std::ostringstream full_message;
	full_message << message << " (line " << line << ").";
	sound->add_error(full_message.str());
}



