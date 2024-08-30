#pragma once

#include <string>
#include <stdint.h>


struct Settings {
	std::string samples_directory = "";
	uint32_t num_voices = 32;
	bool show_voices_used = false;
	std::string tunings_directory = "";
	std::string keyboard_mappings_directory = "";

	std::string errors;

	void	read_settings_files();
	void	read_settings_file(std::string path);
	static std::string	home_path();
	};
extern Settings settings;




