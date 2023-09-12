#pragma once

#include "CLAPExtension.h"
#include <stdint.h>

class CLAPPlugin;
struct clap_host_note_ports;


class CLAPNotePortsExtension : public CLAPExtension {
	public:
		CLAPNotePortsExtension(CLAPPlugin* plugin);

		const char* clap_name();
		const void* clap_extension();

		uint32_t host_supported_dialects();
		void host_rescan(uint32_t flags);

	protected:
		CLAPPlugin* plugin;
		const struct clap_host_note_ports* host_note_ports_extension = nullptr;
	};

