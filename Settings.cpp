#include "Settings.h"
#include "SettingsParser.h"
#include <string_view>
#include <sstream>
#include <fstream>
#include <iostream>
#include <ctype.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>

#define APP_NAME "sfzq"


Settings settings;


void Settings::read_settings_files()
{
	// $XDG_CONFIG_HOME/$app_name/settings is actually the only settings file we
	// read.

	std::string config_dir;
	const char* env_dir = getenv("XDG_CONFIG_HOME");
	if (env_dir)
		config_dir = env_dir;
	else {
		// Default to ~/.config.
		std::string home_dir = home_path();
		if (!home_dir.empty())
			config_dir = home_dir + "/.config";
		}
	if (!config_dir.empty())
		read_settings_file(config_dir + "/" APP_NAME "/settings");
}


static void set_setting(std::string_view setting_name, std::string_view value_token, SettingsParser* parser)
{
	bool ok = true;
	if (setting_name == "samples-directory") {
		settings.samples_directory = SettingsParser::unquote_string(value_token);
		if (!settings.samples_directory.empty() && settings.samples_directory[0] == '~')
			settings.samples_directory = Settings::home_path() + settings.samples_directory.substr(1);
		}
	else if (setting_name == "tunings-directory") {
		settings.tunings_directory = SettingsParser::unquote_string(value_token);
		if (!settings.tunings_directory.empty() && settings.tunings_directory[0] == '~')
			settings.tunings_directory = Settings::home_path() + settings.tunings_directory.substr(1);
		}
	else if (setting_name == "keyboard-mappings-directory") {
		settings.keyboard_mappings_directory = SettingsParser::unquote_string(value_token);
		if (!settings.keyboard_mappings_directory.empty() && settings.keyboard_mappings_directory[0] == '~')
			settings.keyboard_mappings_directory = Settings::home_path() + settings.keyboard_mappings_directory.substr(1);
		}
	else if (setting_name == "num-voices") {
		auto value = SettingsParser::parse_uint32(value_token, &ok);
		if (value > 0)
			settings.num_voices = value;
		}
	else if (setting_name == "show-voices-used")
		settings.show_voices_used = SettingsParser::parse_bool(value_token, &ok);
	else
		parser->errors << "Unknown setting: " << setting_name << "." << std::endl;

	if (!ok)
		parser->errors << "Settings: bad value for \"" << setting_name << "\"";
}

void Settings::read_settings_file(std::string path)
{
	std::fstream file(path);
	if (!file.is_open()) {
		// All settings files are optional, so don't complain if it doesn't
		// exist.
		return;
		}
	std::string contents(
		(std::istreambuf_iterator<char>(file)),
		std::istreambuf_iterator<char>());
	SettingsParser parser(contents.data(), contents.size());
	parser.parse(
		[&](std::string_view setting_name, std::string_view value_token) {
			set_setting(setting_name, value_token, &parser);
			});

	settings.errors += parser.errors.str();
}


std::string Settings::home_path()
{
	const char* home_dir = getenv("HOME");
	if (home_dir == nullptr) {
		const struct passwd* user_info = getpwuid(getuid());
		if (user_info && user_info->pw_dir[0])
			home_dir = user_info->pw_dir;
		}
	if (home_dir == nullptr)
		return "";
	return home_dir;
}



