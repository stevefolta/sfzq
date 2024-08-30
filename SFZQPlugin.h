#include "CLAPPlugin.h"
#include "CairoGUI.h"
#include "CLAPCairoGUIExtension.h"
#include "TuningFile.h"
#include "MessageQueue.h"
#include <string>
#include <thread>
#include <atomic>

class SFZSynth;
class SFZSound;
class Widget;
class FileChooser;
class Label;
class ProgressBar;
class SubsoundWidget;
class TextBox;
class KeyboardWidget;
class Checkbox;


class SFZQPlugin : public CLAPPlugin {
	public:
		SFZQPlugin(const clap_plugin_descriptor_t* descriptor, const clap_host_t* host);
		bool init();
		~SFZQPlugin();

		uint32_t num_note_ports(bool is_input);
		bool get_note_port_info(uint32_t index, bool is_input, clap_note_port_info_t* info_out);
		uint32_t num_audio_ports(bool is_input);
		bool get_audio_port_info(uint32_t index, bool is_input, clap_audio_port_info_t* info_out);

		bool activate(double sample_rate, uint32_t min_frames, uint32_t max_frames);
		void deactivate();
		void reset();
		clap_process_status process(const clap_process_t* process);
		const void* get_extension(const char* id);

		void on_fd(int fd, clap_posix_fd_flags_t flags);
		bool get_gui_size(uint32_t* width_out, uint32_t* height_out);
		bool can_resize_gui() { return true; }
		bool resize_gui(uint32_t width, uint32_t height);
		void paint_gui();
		void mouse_pressed(int32_t x, int32_t y, int button);
		void mouse_released(int32_t x, int32_t y, int button);
		void mouse_moved(int32_t x, int32_t y);
		void key_pressed(std::string_view key);
		void special_key_pressed(std::string_view special_key);
		void main_thread_tick();

		bool save_state(const clap_ostream_t* stream);
		bool load_state(const clap_istream_t* stream);

		// To be called by components.
		void tuning_changed();
		void open_tuning_file_chooser(TuningFile* tuning_file);

	protected:
		enum {
			default_gui_width = 500,
			default_gui_height = 300,
			};

		SFZSynth* synth = nullptr;
		std::string sound_path;
		int sound_subsound = 0;
		SFZSound* loading_sound = nullptr;
		bool initial_load = false;
		std::thread load_samples_thread;
		std::atomic_bool refresh_requested = false;

		Label* filename_label = nullptr;
		ProgressBar* progress_bar = nullptr;;
		FileChooser* file_chooser = nullptr;
		SubsoundWidget* subsound_widget = nullptr;
		TextBox* error_box = nullptr;
		TuningFile tuning, keyboard_mapping;
		TuningFile* cur_tuning_file = nullptr;
		KeyboardWidget* keyboard = nullptr;
		Label* voices_used_label = nullptr;
		Widget* tracking_widget = nullptr;
		uint32_t gui_width = default_gui_width, gui_height = default_gui_height;

		int frames_until_num_voices_update = 0; 	// Audio thread only.

		MessageQueue main_to_audio_queue;
		MessageQueue audio_to_main_queue;
		MessageQueue load_to_main_queue;

		class CairoGUI : public ::CairoGUI {
			public:
				CairoGUI(SFZQPlugin* plugin_in)
					: plugin(plugin_in) {}

				cairo_t* cairo() {
					if (plugin->cairo_gui_extension == nullptr)
						return nullptr;
					return plugin->cairo_gui_extension->cairo;
					}
				void refresh() { plugin->cairo_gui_extension->refresh(); }

				SFZQPlugin* plugin;
			};
		CairoGUI cairo_gui;

		void process_event(const clap_event_header_t* event);
		void process_midi_event(const clap_event_midi_t* event);
		void layout();
		void open_file_chooser();
		void file_chosen(std::string path);
		void file_choice_canceled();
		void tuning_file_chosen(std::string path);
		void tuning_file_choice_canceled();
		void load_sound(std::string path, int subsound = 0);
		void load_samples();
		void load_tuning();
		void send_active_keys();
	};

