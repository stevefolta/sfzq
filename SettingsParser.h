#pragma once

#include <string_view>
#include <functional>
#include <map>
#include <sstream>


class SettingsParser {
	public:
		typedef std::map<std::string_view, std::function<void(std::string_view token, SettingsParser* parser)> > HandlerMap;

		SettingsParser(const char* text, int length, const HandlerMap& handlers_in)
			: p(text), end(text + length), handlers(handlers_in) {}

		void parse();

		std::string	unquote_string(std::string_view token);
		uint32_t	parse_uint32(std::string_view token);
		float	parse_float(std::string_view token);
		bool	parse_bool(std::string_view token);

		std::ostringstream errors;

	protected:
		const char*	p;
		const char*	end;
		const HandlerMap& handlers;

		std::string_view	next_token();
		bool	is_identifier(std::string_view token) { return isalpha(token[0]); }
	};



