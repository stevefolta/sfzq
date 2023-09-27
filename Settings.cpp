#include "Settings.h"
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


class SettingsParser {
	public:
		SettingsParser(const char* text, int length)
			: p(text), end(text + length) {}

		void	parse();

		void	parse_setting(std::string_view setting_name, std::string_view value_token);

	protected:
		const char*	p;
		const char*	end;
		std::ostringstream errors;

		std::string_view	next_token();
		bool	is_identifier(std::string_view token) { return isalpha(token[0]); }
		std::string	unquote_string(std::string_view token);
		uint32_t	parse_uint32(std::string_view token);
		float	parse_float(std::string_view token);
		bool	parse_bool(std::string_view token);
	};


void SettingsParser::parse_setting(std::string_view setting_name, std::string_view value_token)
{
	if (setting_name == "samples-directory") {
		settings.samples_directory = unquote_string(value_token);
		if (!settings.samples_directory.empty() && settings.samples_directory[0] == '~')
			settings.samples_directory = Settings::home_path() + settings.samples_directory.substr(1);
		}
	else if (setting_name == "tunings-directory") {
		settings.tunings_directory = unquote_string(value_token);
		if (!settings.tunings_directory.empty() && settings.tunings_directory[0] == '~')
			settings.tunings_directory = Settings::home_path() + settings.tunings_directory.substr(1);
		}
	else if (setting_name == "num-voices") {
		auto value = parse_uint32(value_token);
		if (value >= 0)
			settings.num_voices = value;
		}
	else if (setting_name == "show-voices-used")
		settings.show_voices_used = parse_bool(value_token);
	else
		errors << "Unknown setting: " << setting_name << "." << std::endl;
}


void SettingsParser::parse()
{
	while (true) {
		std::string_view token = next_token();
		if (token.empty())
			break;
		if (token == "," || token == ";") {
			// We allow commas and semicolons between settings.
			continue;
			}

		// Settings have the form "<name> = <value>".
		if (!is_identifier(token)) {
			std::stringstream message;
			errors << "Error in settings file: not a setting name: \"" << token << '"' << std::endl;
			return;
			}
		std::string_view setting_name = token;
		if (next_token() != "=") {
			errors << "Error in settings file: missing '=' for \"" << setting_name << "\" setting." << std::endl;
			return;
			}
		std::string_view value_token = next_token();
		if (value_token.empty()) {
			errors << "Error in settings file: missing value for \"" << setting_name << "\" setting." << std::endl;
			return;
			}

		// Set the setting.
		parse_setting(setting_name, value_token);
		}

	settings.errors += errors.str();
}


std::string_view SettingsParser::next_token()
{
	char c;

	// Skip whitespace and comments.
	while (true) {
		if (p >= end)
			return "";
		c = *p;
		if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
			p += 1;
		else if (c == '#') {
			// Skip to end of line.
			p += 1;
			while (true) {
				if (p >= end)
					return "";
				c = *p++;
				if (c == '\r' || c == '\n')
					break;
				}
			}
		else
			break;
		}

	const char* token_start = p;
	c = *p++;

	if (c == '"' || c == '\'') {
		// String.
		char quote_char = c;
		while (true) {
			if (p >= end) {
				errors << "Error in settings file: unterminated string." << std::endl;
				return "";
				}
			c = *p++;
			if (c == quote_char)
				break;
			if (c == '\\') {
				// Consume the next character.
				if (p >= end) {
					errors << "Error in settings file: unterminated string." << std::endl;
					return "";
					}
				p += 1;
				}
			}
		}

	else if (isalpha(c)) {
		// Identifier.
		while (p < end) {
			c = *p;
			if (!isalnum(c) && c != '_' && c != '-')
				break;
			p += 1;
			}
		}

	else if (isdigit(c)) {
		// Number.
		if (c == '0' && p < end && (*p == 'x' || *p == 'X')) {
			// Hex number.
			p += 1; 	// Skip the "x".
			if (p >= end || !isxdigit(*p)) {
				errors << "Error in settings file: incomplete hex number." << std::endl;
				return "";
				}
			while (p < end) {
				if (!isxdigit(*p))
					break;
				p += 1;
				}
			}
		else {
			// Decimal number (potentially a float).
			while (p < end) {
				c = *p;
				if (!isdigit(c) && c != '.')
					break;
				p += 1;
				}
			}
		}

	else if (c == '=' || c == ',' || c == ';') {
		// These are single-character tokens.
		}

	else {
		// Unknown.
		errors << "Error in settings file: invalid character: '" << c << "'" << std::endl;
		return "";
		}

	return std::string_view(token_start, p  - token_start);
}


std::string SettingsParser::unquote_string(std::string_view token)
{
	if (token.empty() || (token[0] != '"' && token[0] != '\''))
		return "";

	token = token.substr(1, token.length() - 2);
	std::stringstream result;
	for (auto p = token.begin(); p < token.end(); ++p) {
		char c = *p;
		if (c == '\\')
			p += 1;
		result << c;
		}
	return result.str();
}


uint32_t SettingsParser::parse_uint32(std::string_view token)
{
	char* end_ptr = nullptr;
	uint32_t result = strtoul(std::string(token).c_str(), &end_ptr, 0);
	if (*end_ptr != 0) {
		errors << "Error in settings file: not a number: " << token << std::endl;
		result = 0;
		}
	return result;
}


float SettingsParser::parse_float(std::string_view token)
{
	char* end_ptr = nullptr;
	float result = strtof(std::string(token).c_str(), &end_ptr);
	if (*end_ptr != 0) {
		result = 0.0;
		errors << "Error in settings file: not a floating-point number: " << token << std::endl;
		}
	return result;
}


bool SettingsParser::parse_bool(std::string_view token)
{
	if (token == "true")
		return true;
	else if (token == "false")
		return false;
	errors << "Error in settings file: not a boolean: " << token << std::endl;
	return false;
}



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
	parser.parse();
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



