#pragma once

#include "CLAPExtension.h"


class CLAPParamsExtension : public CLAPExtension {
	public:
		const char* clap_name();
		const void* clap_extension();
	};

