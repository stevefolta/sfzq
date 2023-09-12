#include "CLAPStateExtension.h"
#include "CLAPPlugin.h"
#include "clap/clap.h"


CLAPStateExtension::CLAPStateExtension(CLAPPlugin* plugin_in)
	: plugin(plugin_in)
{
	host_state_extension =
		(const clap_host_state_t*) plugin->host->get_extension(plugin->host, CLAP_EXT_STATE);
}


const char* CLAPStateExtension::clap_name()
{
	return CLAP_EXT_STATE;
}

static const clap_plugin_state_t state_extension = {
	.save = [](const clap_plugin_t* plugin, const clap_ostream_t* stream) {
		return CLAPPlugin::of(plugin)->save_state(stream);
		},
	.load = [](const clap_plugin_t* plugin, const clap_istream_t* stream) {
		return CLAPPlugin::of(plugin)->load_state(stream);
		},
	};

const void* CLAPStateExtension::clap_extension()
{
	return &state_extension;
}


void CLAPStateExtension::host_mark_dirty()
{
	if (host_state_extension && host_state_extension->mark_dirty)
		host_state_extension->mark_dirty(plugin->host);
}



