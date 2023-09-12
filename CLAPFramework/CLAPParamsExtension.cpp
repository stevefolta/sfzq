#include "CLAPParamsExtension.h"
#include "CLAPPlugin.h"
#include "clap/clap.h"


const char* CLAPParamsExtension::clap_name()
{
	return CLAP_EXT_PARAMS;
}


static const clap_plugin_params_t params_extension = {
	.count = [](const clap_plugin_t* plugin) -> uint32_t {
		return CLAPPlugin::of(plugin)->num_params();
		},
	.get_info = [](const clap_plugin_t* plugin, uint32_t param_index, clap_param_info_t* param_info_out) -> bool {
		return CLAPPlugin::of(plugin)->get_param_info(param_index, param_info_out);
		},
	.get_value = [](const clap_plugin_t* plugin, clap_id param_id, double* value_out) -> bool {
		return CLAPPlugin::of(plugin)->get_param_value(param_id, value_out);
		},
	.value_to_text = [](const clap_plugin_t* plugin, clap_id param_id, double value, char* out_buffer, uint32_t out_buffer_capacity) {
		return CLAPPlugin::of(plugin)->param_value_to_text(param_id, value, out_buffer, out_buffer_capacity);
		},
	.text_to_value = [](const clap_plugin_t* plugin, clap_id param_id, const char* param_value_text, double* value_out) {
		return CLAPPlugin::of(plugin)->param_text_to_value(param_id, param_value_text, value_out);
		},
	.flush = [](const clap_plugin_t* plugin, const clap_input_events_t* in, const clap_output_events_t* out) {
		CLAPPlugin::of(plugin)->flush_params(in, out);
		}
	};

const void* CLAPParamsExtension::clap_extension()
{
	return &params_extension;
}


