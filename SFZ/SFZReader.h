#pragma once

#include <string>

class SFZRegion;
class SFZSound;


class SFZReader {
	public:
		SFZReader(SFZSound* sound);
		~SFZReader();

		void read(std::string path);
		void read(const char* text, unsigned int length);

	protected:
		SFZSound* sound;
		int line;

		const char* handle_line_end(const char* p);
		const char* read_path_into(std::string* pathOut, const char* p, const char* end);
		int key_value(const std::string& str);
		int trigger_value(const std::string& str);
		int loop_mode_value(const std::string& str);
		void finish_region(SFZRegion* region);
		void error(const std::string& message);
	};


