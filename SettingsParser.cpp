#include "SettingsParser.h"


void SettingsParser::parse(const SettingHandler& handler)
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
		handler(setting_name, value_token);
		}
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


std::string SettingsParser::unquote_string(std::string_view token, bool* ok)
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


std::string SettingsParser::quote_string(std::string_view str)
{
	std::stringstream result;
	result << '"';
	for (auto p = str.begin(); p < str.end(); ++p) {
		char c = *p;
		if (c == '"' || c == '\\')
			result << '\\';
		result << c;
		}
	result << '"';
	return result.str();
}


uint32_t SettingsParser::parse_uint32(std::string_view token, bool* ok)
{
	char* end_ptr = nullptr;
	uint32_t result = strtoul(std::string(token).c_str(), &end_ptr, 0);
	if (*end_ptr != 0) {
		result = 0;
		if (ok)
			*ok = false;
		}
	return result;
}


float SettingsParser::parse_float(std::string_view token, bool* ok)
{
	char* end_ptr = nullptr;
	float result = strtof(std::string(token).c_str(), &end_ptr);
	if (*end_ptr != 0) {
		result = 0.0;
		if (ok)
			*ok = false;
		}
	return result;
}


bool SettingsParser::parse_bool(std::string_view token, bool* ok)
{
	if (token == "true")
		return true;
	else if (token == "false")
		return false;
	if (ok)
		*ok = false;
	return false;
}



