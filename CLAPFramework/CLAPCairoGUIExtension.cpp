#include "CLAPCairoGUIExtension.h"
#include "CLAPPlugin.h"
#include "CLAPPosixFDExtension.h"
#include <X11/Xutil.h>
#include <iostream>


CLAPCairoGUIExtension::~CLAPCairoGUIExtension()
{
	destroy_gui();
}


const char* CLAPCairoGUIExtension::clap_name()
{
	return CLAP_EXT_GUI;
}


inline CLAPCairoGUIExtension* get_gui(const clap_plugin_t* plugin)
{
	return CLAPPlugin::of(plugin)->cairo_gui_extension;
}

static const clap_plugin_gui_t gui_extension = {
	.is_api_supported = [](const clap_plugin_t* plugin, const char* api, bool is_floating) -> bool {
		return strcmp(api, CLAP_WINDOW_API_X11) == 0 && !is_floating;
		},
	.get_preferred_api = [](const clap_plugin_t* plugin, const char** api_out, bool* is_floating_out) -> bool {
		*api_out = CLAP_WINDOW_API_X11;
		*is_floating_out = false;
		return true;
		},
	.create = [](const clap_plugin_t* plugin, const char* api, bool is_floating) -> bool {
		if (!gui_extension.is_api_supported(plugin, api, is_floating))
			return false;
		return get_gui(plugin)->create_gui();
		},
	.destroy = [](const clap_plugin_t* plugin) {
		return get_gui(plugin)->destroy_gui();
		},
	.set_scale = [](const clap_plugin_t* plugin, double scale) {
		return false;
		},
	.get_size = [](const clap_plugin_t* plugin, uint32_t* width_out, uint32_t* height_out) -> bool {
		return get_gui(plugin)->get_gui_size(width_out, height_out);
		},
	.can_resize = [](const clap_plugin_t* plugin) -> bool {
		return get_gui(plugin)->can_resize();
		},
	.get_resize_hints = [](const clap_plugin_t* plugin, clap_gui_resize_hints_t* hints_out) -> bool {
		return false;
		},
	.adjust_size = [](const clap_plugin_t* plugin, uint32_t* width_out, uint32_t* height_out) -> bool {
		return true;
		},
	.set_size = [](const clap_plugin_t* plugin, uint32_t width, uint32_t height) -> bool {
		return get_gui(plugin)->resize(width, height);
		},
	.set_parent = [](const clap_plugin_t* plugin, const clap_window_t* window) -> bool {
		return get_gui(plugin)->set_gui_parent(window);
		},
	.set_transient = [](const clap_plugin_t* plugin, const clap_window_t* window) -> bool {
		return false;
		},
	.suggest_title = [](const clap_plugin_t* plugin, const char* title) {},
	.show = [](const clap_plugin_t* plugin) -> bool {
		return get_gui(plugin)->show_gui();
		},
	.hide = [](const clap_plugin_t* plugin) -> bool {
		return get_gui(plugin)->hide_gui();
		},
	};

const void* CLAPCairoGUIExtension::clap_extension()
{
	return &gui_extension;
}


void CLAPCairoGUIExtension::refresh()
{
	plugin->paint_gui();
	put_image();
}


bool CLAPCairoGUIExtension::create_gui()
{
	display = XOpenDisplay(nullptr);
	if (display == nullptr)
		return false;

	width = default_gui_width;
	height = default_gui_height;
	plugin->get_gui_size(&width, &height);

	// Create the window.
	XSetWindowAttributes attributes = {};
	window =
		XCreateWindow(
			display, DefaultRootWindow(display),
			0, 0, width, height,
			0, 0, InputOutput, CopyFromParent,
			CWOverrideRedirect, &attributes);
	Atom embed_info_atom = XInternAtom(display, "_XEMBED_INFO", 0);
	uint32_t embed_info_data[] = {
		0, 	// version
		0, 	// not mapped
		};
	XChangeProperty(
		display, window,
		embed_info_atom, embed_info_atom, 32, PropModeReplace,
		(uint8_t*) embed_info_data, 2);
	XSizeHints size_hints = {};
	size_hints.flags = PMinSize | PMaxSize;
	size_hints.min_width = size_hints.max_width = width;
	size_hints.min_height = size_hints.max_height = height;
	XSetWMNormalHints(display, window, &size_hints);
	XStoreName(display, window, plugin->clap_plugin.desc->name);
	XSelectInput(
		display, window,
		SubstructureNotifyMask | ExposureMask | PointerMotionMask |
			ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask |
			StructureNotifyMask | EnterWindowMask | LeaveWindowMask | ButtonMotionMask |
			KeymapStateMask | FocusChangeMask | PropertyChangeMask);

	// Create Cairo stuff.
	surface = cairo_xlib_surface_create(display, window, DefaultVisual(display, 0), width, height);
	cairo = cairo_create(surface);

	// Register to get events.
	plugin->posix_fd_extension->register_fd(ConnectionNumber(display), CLAP_POSIX_FD_READ);

	return true;
}

void CLAPCairoGUIExtension::destroy_gui()
{
	// Unregister events.
	if (display && plugin->posix_fd_extension)
		plugin->posix_fd_extension->unregister_fd(ConnectionNumber(display));

	// Clean up.
	if (cairo) {
		cairo_destroy(cairo);
		cairo = nullptr;
		}
	if (surface) {
		cairo_surface_destroy(surface);
		surface = nullptr;
		}
	if (display) {
		XDestroyWindow(display, window);
		XCloseDisplay(display);
		display = nullptr;
		}
}

bool CLAPCairoGUIExtension::get_gui_size(uint32_t* width_out, uint32_t* height_out)
{
	return plugin->get_gui_size(width_out, height_out);
}

bool CLAPCairoGUIExtension::set_gui_parent(const clap_window_t* parent_window)
{
	XReparentWindow(display, window, parent_window->x11, 0, 0);
	XFlush(display);
	return true;
}

bool CLAPCairoGUIExtension::show_gui()
{
	XMapRaised(display, window);
	XFlush(display);
	return true;
}

bool CLAPCairoGUIExtension::hide_gui()
{
	XUnmapWindow(display, window);
	XFlush(display);
	return true;
}

bool CLAPCairoGUIExtension::can_resize()
{
	return plugin->can_resize_gui();
}

bool CLAPCairoGUIExtension::resize(uint32_t width, uint32_t height)
{
	if (!plugin->resize_gui(width, height))
		return false;

	XResizeWindow(display, window, width, height);
	cairo_xlib_surface_set_size(surface, width, height);
	XFlush(display);
	this->width = width;
	this->height = height;
	refresh();
	return true;
}


void CLAPCairoGUIExtension::on_fd(int fd, clap_posix_fd_flags_t flags)
{
	XFlush(display);

	if (XPending(display)) {
		XEvent event;
		XNextEvent(display, &event);

		while (XPending(display)) {
			XEvent next_event;
			XNextEvent(display, &next_event);

			if (event.type == MotionNotify && next_event.type == MotionNotify) {
				// Merge adjacent mouse motion events.
				}
			else {
				process_x11_event(&event);
				XFlush(display);
				}

			event = next_event;
			}

		process_x11_event(&event);
		XFlush(display);
		plugin->paint_gui();
		put_image();
		}
}


void CLAPCairoGUIExtension::process_x11_event(XEvent* event)
{
	switch (event->type) {
		case Expose:
			if (event->xexpose.window == window) {
				if (needs_initial_paint) {
					plugin->paint_gui();
					needs_initial_paint = false;
					}
				put_image();
				}
			break;

		case ButtonPress:
			if (event->xbutton.window == window)
				plugin->mouse_pressed(event->xbutton.x, event->xbutton.y, event->xbutton.button);
			break;
		case ButtonRelease:
			if (event->xbutton.window == window)
				plugin->mouse_released(event->xbutton.x, event->xbutton.y, event->xbutton.button);
			break;
		case MotionNotify:
			if (event->xmotion.window == window)
				plugin->mouse_moved(event->xmotion.x, event->xmotion.y);
			break;
		}
}

void CLAPCairoGUIExtension::put_image()
{
	cairo_surface_flush(surface);
}



