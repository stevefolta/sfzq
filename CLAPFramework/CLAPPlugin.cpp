#include "CLAPPlugin.h"
#include <string_view>
#include <iostream>


static const clap_plugin_t clap_plugin_template = {
	.desc = nullptr,
	.plugin_data = nullptr,

	.init = [](const clap_plugin* plugin) -> bool {
		return ((CLAPPlugin*) plugin->plugin_data)->init();
		},
	.destroy = [](const clap_plugin* plugin) {
		delete (CLAPPlugin*) plugin->plugin_data;
		},
	.activate = [](const clap_plugin* plugin, double sample_rate, uint32_t min_frames, uint32_t max_frames) -> bool {
		return ((CLAPPlugin*) plugin->plugin_data)->activate(sample_rate, min_frames, max_frames);
		},
	.deactivate = [](const clap_plugin* plugin) {
		((CLAPPlugin*) plugin->plugin_data)->deactivate();
		},
	.start_processing = [](const clap_plugin* plugin) -> bool {
		return ((CLAPPlugin*) plugin->plugin_data)->start_processing();
		},
	.stop_processing = [](const clap_plugin* plugin) {
		((CLAPPlugin*) plugin->plugin_data)->stop_processing();
		},
	.reset = [](const clap_plugin_t* plugin) {
		((CLAPPlugin*) plugin->plugin_data)->reset();
		},
	.process = [](const clap_plugin* plugin, const clap_process_t* process) -> clap_process_status {
		return ((CLAPPlugin*) plugin->plugin_data)->process(process);
		},
	.get_extension = [](const clap_plugin* plugin, const char* id) -> const void* {
		return ((CLAPPlugin*) plugin->plugin_data)->get_extension(id);
		},
	.on_main_thread = [](const clap_plugin* plugin) {
		((CLAPPlugin*) plugin->plugin_data)->main_thread_tick();
		},
	};


CLAPPlugin::CLAPPlugin(const clap_plugin_descriptor_t* descriptor, const clap_host_t* host_in)
	: clap_plugin(clap_plugin_template), host(host_in)
{
	clap_plugin.desc = descriptor;
	clap_plugin.plugin_data = this;
}



CLAPPluginFactory::CLAPPluginFactory()
{
	augmented_factory.self = this;
	augmented_factory.clap_factory.get_plugin_count =
		[](const clap_plugin_factory_t* clap_factory) -> uint32_t {
			return ((AugmentedFactory*) clap_factory)->self->descriptors().size();
			};
	augmented_factory.clap_factory.get_plugin_descriptor =
		[](const clap_plugin_factory_t* clap_factory, uint32_t index) -> const clap_plugin_descriptor_t* {
			auto& descriptors = ((AugmentedFactory*) clap_factory)->self->descriptors();
			if (index >= descriptors.size())
				return nullptr;
			return &descriptors[index];
			};
	augmented_factory.clap_factory.create_plugin =
		[](const clap_plugin_factory_t* clap_factory, const clap_host_t* host, const char* plugin_id) -> const clap_plugin_t* {
			CLAPPluginFactory* self = ((AugmentedFactory*) clap_factory)->self;
			auto& descriptors = self->descriptors();
			std::string_view plugin_id_view(plugin_id);
			for (const auto& descriptor: descriptors) {
				if (plugin_id_view == descriptor.id)
					return &self->create_plugin(&descriptor, host)->clap_plugin;
				}
			return nullptr;
			};
}


/*
MIT License

Copyright (c) 2023 Steve Folta

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

