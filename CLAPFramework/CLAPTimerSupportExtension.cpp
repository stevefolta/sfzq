#include "CLAPTimerSupportExtension.h"
#include "CLAPPlugin.h"


CLAPTimerSupportExtension::CLAPTimerSupportExtension(CLAPPlugin* plugin_in)
	: plugin(plugin_in)
{
	host_extension =
		(const clap_host_timer_support_t*) plugin->host->get_extension(plugin->host, CLAP_EXT_TIMER_SUPPORT);
}


const char* CLAPTimerSupportExtension::clap_name()
{
	return CLAP_EXT_TIMER_SUPPORT;
}

static const clap_plugin_timer_support_t timer_support_extension = {
	.on_timer = [](const clap_plugin_t* plugin, clap_id timer_id) {
		CLAPPlugin::of(plugin)->timer_tick(timer_id);
		},
	};

const void* CLAPTimerSupportExtension::clap_extension()
{
	return &timer_support_extension;
}


bool CLAPTimerSupportExtension::host_register_timer(uint32_t period_ms, clap_id* timer_id_out)
{
	if (host_extension && host_extension->register_timer)
		return host_extension->register_timer(plugin->host, period_ms, timer_id_out);
	return false;
}

bool CLAPTimerSupportExtension::host_unregister_timer(clap_id timer_id)
{
	if (host_extension && host_extension->unregister_timer)
		return host_extension->unregister_timer(plugin->host, timer_id);
	return false;
}


