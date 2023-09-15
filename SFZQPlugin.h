#include "CLAPPlugin.h"
#include "CairoGUI.h"
#include "CLAPCairoGUIExtension.h"
#include "MessageQueue.h"
#include <string>
#include <thread>

class SFZSynth;
class SFZSound;
class Widget;
class FileChooser;
class Label;
class ProgressBar;


class SFZQPlugin : public CLAPPlugin {
	public:
		SFZQPlugin(const clap_plugin_descriptor_t* descriptor, const clap_host_t* host);
		~SFZQPlugin();

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
		void main_thread_tick();

		bool save_state(const clap_ostream_t* stream);
		bool load_state(const clap_istream_t* stream);

	protected:
		enum {
			default_gui_width = 500,
			default_gui_height = 300,
			};

		SFZSynth* synth = nullptr;
		SFZSound* loading_sound = nullptr;
		std::thread load_samples_thread;

		Label* filename_label = nullptr;
		ProgressBar* progress_bar = nullptr;;
		FileChooser* file_chooser = nullptr;
		Widget* tracking_widget = nullptr;
		uint32_t gui_width = default_gui_width, gui_height = default_gui_height;

		MessageQueue main_to_audio_queue;
		MessageQueue audio_to_main_queue;
		MessageQueue load_to_main_queue;

		class CairoGUI : public ::CairoGUI {
			public:
				CairoGUI(SFZQPlugin* plugin_in)
					: plugin(plugin_in) {}

				cairo_t* cairo() { return plugin->cairo_gui_extension->cairo; }
				void refresh() { plugin->cairo_gui_extension->refresh(); }

				SFZQPlugin* plugin;
			};
		CairoGUI cairo_gui;

		void process_event(const clap_event_header_t* event);
		void layout();
		void open_file_chooser();
		void load_sfx(std::string path);
		void load_samples();
	};

