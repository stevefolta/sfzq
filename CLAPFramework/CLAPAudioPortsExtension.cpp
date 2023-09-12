#include "CLAPAudioPortsExtension.h"
#include "CLAPPlugin.h"


CLAPAudioPortsExtension::CLAPAudioPortsExtension(CLAPPlugin* plugin_in)
	: plugin(plugin_in)
{
	host_audio_ports_extension =
		(const clap_host_audio_ports_t*) plugin->host->get_extension(plugin->host, CLAP_EXT_AUDIO_PORTS);
}


const char* CLAPAudioPortsExtension::clap_name()
{
	return CLAP_EXT_AUDIO_PORTS;
}

static const clap_plugin_audio_ports_t audio_ports_extension = {
	.count = [](const clap_plugin_t* plugin, bool is_input) -> uint32_t {
		return CLAPPlugin::of(plugin)->num_audio_ports(is_input);
		},
	.get = [](const clap_plugin_t* plugin, uint32_t index, bool is_input, clap_audio_port_info_t* info_out) -> bool {
		return CLAPPlugin::of(plugin)->get_audio_port_info(index, is_input, info_out);
		},
	};

const void* CLAPAudioPortsExtension::clap_extension()
{
	return &audio_ports_extension;
}


bool CLAPAudioPortsExtension::host_is_rescan_flag_supported(uint32_t flag)
{
	if (host_audio_ports_extension && host_audio_ports_extension->is_rescan_flag_supported)
		return host_audio_ports_extension->is_rescan_flag_supported(plugin->host, flag);
	return false;
}

void CLAPAudioPortsExtension::host_rescan(uint32_t flags)
{
	if (host_audio_ports_extension && host_audio_ports_extension->rescan)
		return host_audio_ports_extension->rescan(plugin->host, flags);
}



