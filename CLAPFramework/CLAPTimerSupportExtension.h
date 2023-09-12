#pragma once

#include "CLAPExtension.h"
#include "clap/clap.h"
#include <stdint.h>

class CLAPPlugin;
struct clap_host_timer_support;


class CLAPTimerSupportExtension : public CLAPExtension {
	public:
		CLAPTimerSupportExtension(CLAPPlugin* plugin);

		const char* clap_name();
		const void* clap_extension();

		bool host_register_timer(uint32_t period_ms, clap_id* timer_id_out);
		bool host_unregister_timer(clap_id timer_id);

	protected:
		CLAPPlugin* plugin;
		const struct clap_host_timer_support* host_extension;
	};


