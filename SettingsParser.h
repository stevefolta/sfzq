#pragma once

#include <string_view>
#include <functional>
#include <map>
#include <sstream>


class SettingsParser {
	public:
		typedef std::function<void(std::string_view setting, std::string_view value_token)> SettingHandler;

		SettingsParser(const char* text, int length)
			: p(text), end(text + length) {}

		void parse(const SettingHandler& handler);

		static std::string unquote_string(std::string_view token, bool* ok = nullptr);
		static std::string quote_string(std::string_view str);
		static uint32_t	parse_uint32(std::string_view token, bool* ok = nullptr);
		static float	parse_float(std::string_view token, bool* ok = nullptr);
		static bool	parse_bool(std::string_view token, bool* ok = nullptr);

		std::ostringstream errors;

	protected:
		const char*	p;
		const char*	end;

		std::string_view	next_token();
		bool	is_identifier(std::string_view token) { return isalpha(token[0]); }
	};



