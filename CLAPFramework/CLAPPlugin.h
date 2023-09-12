#pragma once

#include "clap/clap.h"
#include <vector>

class CLAPPosixFDExtension;
class CLAPCairoGUIExtension;
class CLAPAudioPortsExtension;
class CLAPNotePortsExtension;
class CLAPParamsExtension;
class CLAPStateExtension;
class CLAPTimerSupportExtension;


class CLAPPlugin {
	public:
		CLAPPlugin(const clap_plugin_descriptor_t* descriptor, const clap_host_t* host);
		virtual ~CLAPPlugin() {}

		virtual bool init() { return true; }
		virtual bool activate(double sample_rate, uint32_t min_frames, uint32_t max_frames)
			{ return true; }
		virtual void deactivate() {}
		virtual bool start_processing() { return true; }
		virtual void stop_processing() {}
		virtual void reset() {}
		virtual clap_process_status process(const clap_process_t* process) {
			return CLAP_PROCESS_CONTINUE;
			}
		virtual const void* get_extension(const char* id) {
			return nullptr;
			}
		virtual void main_thread_tick() {}

		// Extensions.
		CLAPPosixFDExtension* posix_fd_extension = nullptr;
		virtual void on_fd(int fd, clap_posix_fd_flags_t flags) {}
		CLAPCairoGUIExtension* cairo_gui_extension = nullptr;
		virtual bool get_gui_size(uint32_t* width_out, uint32_t* height_out) { return false; }
		virtual void paint_gui() {}
		virtual void mouse_pressed(int32_t x, int32_t y, int button) {}
		virtual void mouse_released(int32_t x, int32_t y, int button) {}
		virtual void mouse_moved(int32_t x, int32_t y) {}
		virtual bool can_resize_gui() { return false; }
		virtual bool resize_gui(uint32_t width, uint32_t height) { return false; }
		CLAPAudioPortsExtension* audio_ports_extension = nullptr;
		virtual uint32_t num_audio_ports(bool is_input) { return 0; }
		virtual bool get_audio_port_info(uint32_t index, bool is_input, clap_audio_port_info_t* info_out) {
			return false;
			}
		CLAPNotePortsExtension* note_ports_extension = nullptr;
		virtual uint32_t num_note_ports(bool is_input) { return 0; }
		virtual bool get_note_port_info(uint32_t index, bool is_input, clap_note_port_info_t* info_out) {
			return false;
			}
		CLAPParamsExtension* params_extension = nullptr;
		virtual uint32_t num_params() { return 0; }
		virtual bool get_param_info(uint32_t param_index, clap_param_info_t* param_info_out) { return false; }
		virtual bool get_param_value(clap_id param_id, double* value_out) { return false; }
		virtual bool param_value_to_text(clap_id param_id, double value, char* out_buffer, uint32_t out_buffer_capacity) {
			return false;
			}
		virtual bool param_text_to_value(clap_id param_id, const char* param_value_text, double* value_out) {
			return false;
			}
		virtual void flush_params(const clap_input_events_t* in, const clap_output_events_t* out) {}
		CLAPStateExtension* state_extension = nullptr;
		virtual bool save_state(const clap_ostream_t* stream) { return false; }
		virtual bool load_state(const clap_istream_t* stream) { return false; }
		CLAPTimerSupportExtension* timer_support_extension = nullptr;
		virtual void timer_tick(clap_id timer_id) {}

		static CLAPPlugin* of(const clap_plugin_t* clap_plugin) {
			return (CLAPPlugin*) clap_plugin->plugin_data;
			}

		clap_plugin_t clap_plugin;
		const clap_host_t* host;
	};


class CLAPPluginFactory {
	public:
		CLAPPluginFactory();

		virtual bool init() { return true; }
		virtual void deinit() {}

		virtual const std::vector<clap_plugin_descriptor_t>& descriptors() = 0;
		virtual CLAPPlugin* create_plugin(const clap_plugin_descriptor_t* descriptor, const clap_host_t* host) = 0;

		const clap_plugin_factory_t* clap_factory() { return &augmented_factory.clap_factory; }

	protected:
		// clap_plugin_factory_t doesn't have a "factory_data" field, so we have to
		// do this:
		struct AugmentedFactory {
			clap_plugin_factory_t clap_factory;
			CLAPPluginFactory* self;
			};
		AugmentedFactory augmented_factory;
	};


#define DECLARE_CLAP_PLUGIN_ENTRY(factory) 	\
	extern "C" const clap_plugin_entry_t clap_entry = { 	\
		.clap_version = CLAP_VERSION_INIT, 	\
		.init = [](const char* path) -> bool { 	\
			return (factory).init(); 	\
			}, 	\
		.deinit = []() { (factory).deinit(); }, 	\
		.get_factory = [](const char* factory_id) -> const void* { 	\
			if (strcmp(factory_id, CLAP_PLUGIN_FACTORY_ID) == 0) 	\
				return (factory).clap_factory(); 	\
			return nullptr; 	\
			} 	\
		};


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

