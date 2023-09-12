#pragma once

#include <string.h>


class CLAPExtension {
	public:
		virtual ~CLAPExtension() {}
		virtual const char* clap_name() = 0;
		virtual const void* clap_extension() = 0;

		const void* for_name(const char* name) {
			return (strcmp(name, clap_name()) == 0) ? clap_extension() : nullptr;
			}
	};


