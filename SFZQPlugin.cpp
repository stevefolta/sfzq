#include "SFZQPlugin.h"
#include "SFZSynth.h"
#include "Button.h"
#include "FileChooser.h"
#include "Label.h"
#include "ProgressBar.h"
#include "CLAPPosixFDExtension.h"
#include "CLAPCairoGUIExtension.h"
#include "CLAPAudioPortsExtension.h"
#include "CLAPNotePortsExtension.h"
#include "CLAPParamsExtension.h"
#include "CLAPStateExtension.h"
#include "CLAPStream.h"


SFZQPlugin::SFZQPlugin(const clap_plugin_descriptor_t* descriptor, const clap_host_t* host)
	: CLAPPlugin(descriptor, host), cairo_gui(this)
{
	posix_fd_extension = new CLAPPosixFDExtension(this);
	cairo_gui_extension = new CLAPCairoGUIExtension(this);
	audio_ports_extension = new CLAPAudioPortsExtension(this);
	note_ports_extension = new CLAPNotePortsExtension(this);
	params_extension = new CLAPParamsExtension();
	state_extension = new CLAPStateExtension(this);

	synth = new SFZSynth(32);

	// GUI.
	/***/
}

SFZQPlugin::~SFZQPlugin()
{
	/***/

	delete synth;

	delete cairo_gui_extension;
	delete posix_fd_extension;
	delete audio_ports_extension;
	delete note_ports_extension;
	delete params_extension;
	delete state_extension;
}


bool SFZQPlugin::activate(double sample_rate, uint32_t min_frames, uint32_t max_frames)
{
	synth->set_sample_rate(sample_rate);
	return true;
}

void SFZQPlugin::deactivate()
{
}

void SFZQPlugin::reset()
{
	synth->reset();
}


clap_process_status SFZQPlugin::process(const clap_process_t* process)
{
	/***/
	return CLAP_PROCESS_CONTINUE;
}


const void* SFZQPlugin::get_extension(const char* id)
{
	const void* extension = nullptr;
	if ((extension = posix_fd_extension->for_name(id))) ;
	else if ((extension = cairo_gui_extension->for_name(id))) ;
	else if ((extension = audio_ports_extension->for_name(id))) ;
	else if ((extension = note_ports_extension->for_name(id))) ;
	else if ((extension = params_extension->for_name(id))) ;
	else if ((extension = state_extension->for_name(id))) ;
	return extension;
}


void SFZQPlugin::on_fd(int fd, clap_posix_fd_flags_t flags)
{
	cairo_gui_extension->on_fd(fd, flags);
}


bool SFZQPlugin::get_gui_size(uint32_t* width_out, uint32_t* height_out)
{
	*width_out = gui_width;
	*height_out = gui_height;
	return true;
}


bool SFZQPlugin::resize_gui(uint32_t width, uint32_t height)
{
	/***/
	return true;
}


void SFZQPlugin::paint_gui()
{
	auto cairo = cairo_gui_extension->cairo;
	cairo_push_group(cairo);

	// Background.
	cairo_set_source_rgb(cairo, 1.0, 1.0, 1.0);
	cairo_paint(cairo);

	/***/

	// Blit to screen.
	cairo_pop_group_to_source(cairo);
	cairo_paint(cairo);
}


void SFZQPlugin::mouse_pressed(int32_t x, int32_t y, int button)
{
	if (button == Button4 || button == Button5) {
		if (file_chooser->contains(x, y)) {
			if (button == Button5)
				file_chooser->scroll_down(x, y);
			else
				file_chooser->scroll_up(x, y);
			}
		return;
		}
	else if (button != Button1)
		return;

	if (file_chooser->contains(x, y))
		tracking_widget = file_chooser;
	if (tracking_widget)
		tracking_widget->mouse_pressed(x, y);
	cairo_gui_extension->refresh();
}

void SFZQPlugin::mouse_released(int32_t x, int32_t y, int button)
{
	if (button != 1)
		return;

	if (tracking_widget && tracking_widget->mouse_released(x, y)) {
		if (tracking_widget == load_button) {
			//*** TODO
			}
		}
	tracking_widget = nullptr;
	cairo_gui_extension->refresh();
}

void SFZQPlugin::mouse_moved(int32_t x, int32_t y)
{
	if (tracking_widget)
		tracking_widget->mouse_moved(x, y);
}


void SFZQPlugin::main_thread_tick()
{
	/***/
}


bool SFZQPlugin::save_state(const clap_ostream_t* stream)
{
	/***/
	return true;
}

bool SFZQPlugin::load_state(const clap_istream_t* stream)
{
	/***/
	return true;
}



