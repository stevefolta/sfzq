#pragma once

#include "CLAPExtension.h"
#include <stdint.h>

class CLAPPlugin;
struct clap_host_audio_ports;


class CLAPAudioPortsExtension : public CLAPExtension {
	public:
		CLAPAudioPortsExtension(CLAPPlugin* plugin_in);

		const char* clap_name();
		const void* clap_extension();

		bool host_is_rescan_flag_supported(uint32_t flag);
		void host_rescan(uint32_t flags);

	protected:
		CLAPPlugin* plugin;
		const struct clap_host_audio_ports* host_audio_ports_extension = nullptr;
	};

