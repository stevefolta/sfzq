#pragma once

#include "CLAPExtension.h"
#include <X11/Xlib.h>
#include <cairo/cairo-xlib.h>
#include "clap/clap.h"
#include <stdint.h>

class CLAPPlugin;


class CLAPCairoGUIExtension : public CLAPExtension {
	public:
		CLAPCairoGUIExtension(CLAPPlugin* plugin_in)
			: plugin(plugin_in) {}
		~CLAPCairoGUIExtension();

		const char* clap_name();
		const void* clap_extension();

		void refresh();

		bool create_gui();
		void destroy_gui();
		bool get_gui_size(uint32_t* width_out, uint32_t* height_out);
		bool set_gui_parent(const clap_window_t* parent_window);
		bool show_gui();
		bool hide_gui();
		bool can_resize();
		bool resize(uint32_t width, uint32_t height);
		void on_fd(int fd, clap_posix_fd_flags_t flags);

		cairo_t* cairo = nullptr;

		uint32_t width, height;

	protected:
		enum {
			default_gui_width = 300,
			default_gui_height = 200,
			};

		CLAPPlugin* plugin;
		Display* display = nullptr;
		Window window;
		cairo_surface_t* surface = nullptr;
		bool needs_initial_paint = true;

		void process_x11_event(XEvent* event);
		void handle_key_event(XKeyEvent* event);
		void put_image();
	};


