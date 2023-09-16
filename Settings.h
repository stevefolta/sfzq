#pragma once

#include <string>


struct Settings {
	std::string samples_directory = "";
	uint32_t num_voices = 32;

	void	read_settings_files();
	void	read_settings_file(std::string path);
	static std::string	home_path();
	};
extern Settings settings;



