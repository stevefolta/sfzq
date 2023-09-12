#pragma once

#include "CLAPExtension.h"

class CLAPPlugin;
struct clap_host_state;


class CLAPStateExtension : public CLAPExtension {
	public:
		CLAPStateExtension(CLAPPlugin* plugin);

		const char* clap_name();
		const void* clap_extension();

		void host_mark_dirty();

	protected:
		CLAPPlugin* plugin;
		const struct clap_host_state* host_state_extension;
	};

