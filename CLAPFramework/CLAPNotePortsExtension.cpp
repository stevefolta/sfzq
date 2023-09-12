#include "CLAPNotePortsExtension.h"
#include "CLAPPlugin.h"
#include "clap/clap.h"


CLAPNotePortsExtension::CLAPNotePortsExtension(CLAPPlugin* plugin_in)
	: plugin(plugin_in)
{
	host_note_ports_extension =
		(const clap_host_note_ports_t*) plugin->host->get_extension(plugin->host, CLAP_EXT_NOTE_PORTS);
}


const char* CLAPNotePortsExtension::clap_name()
{
	return CLAP_EXT_NOTE_PORTS;
}

static const clap_plugin_note_ports_t note_ports_extension = {
	.count = [](const clap_plugin_t* plugin, bool is_input) {
		return CLAPPlugin::of(plugin)->num_note_ports(is_input);
		},
	.get = [](const clap_plugin_t* plugin, uint32_t index, bool is_input, clap_note_port_info_t* info_out) {
		return CLAPPlugin::of(plugin)->get_note_port_info(index, is_input, info_out);
		},
	};

const void* CLAPNotePortsExtension::clap_extension()
{
	return &note_ports_extension;
}


uint32_t CLAPNotePortsExtension::host_supported_dialects()
{
	if (host_note_ports_extension && host_note_ports_extension->supported_dialects)
		return host_note_ports_extension->supported_dialects(plugin->host);
	return 0;
}

void CLAPNotePortsExtension::host_rescan(uint32_t flags)
{
	if (host_note_ports_extension && host_note_ports_extension->rescan)
		host_note_ports_extension->rescan(plugin->host, flags);
}



