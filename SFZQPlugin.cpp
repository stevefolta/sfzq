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
#include "CLAPOutBuffer.h"


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
	const uint32_t num_frames = process->frames_count;
	const uint32_t num_events = process->in_events->size(process->in_events);
	uint32_t cur_event = 0;
	uint32_t next_event_frame = num_events > 0 ? 0 : num_frames;

	// If we had parameters, this is where we'd send parameter changes from the
	// GUI back to the host.

	// Setup rendering.
	CLAPOutBuffer out_buffer(&process->audio_outputs[0]);

	// Render and handle events.
	for (uint32_t cur_frame = 0; cur_frame < num_frames; ) {
		// Handle events at this frame (and/or update next_event_frame).
		while (cur_event < num_events && next_event_frame == cur_frame) {
			const clap_event_header_t* event = process->in_events->get(process->in_events, cur_event);
			if (event->time != cur_frame) {
				next_event_frame = event->time;
				break;
				}

			process_event(event);

			cur_event += 1;
			if (cur_event == num_events) {
				next_event_frame = num_frames;
				break;
				}
			}

		// Render.
		synth->render(&out_buffer, cur_frame, next_event_frame - cur_frame);
		cur_frame = next_event_frame;
		}

	// Send note-end events back to the host.
	//*** TODO

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


void SFZQPlugin::process_event(const clap_event_header_t* event)
{
	switch (event->type) {
		case CLAP_EVENT_NOTE_ON:
			{
			auto note_event = (const clap_event_note_t*) event;
			synth->note_on(note_event->key, note_event->velocity);
			}
			break;
		case CLAP_EVENT_NOTE_OFF:
		case CLAP_EVENT_NOTE_CHOKE:
			{
			auto note_event = (const clap_event_note_t*) event;
			synth->note_off(
				note_event->key, note_event->velocity,
				(event->type == CLAP_EVENT_NOTE_OFF));
			}
			break;
		case CLAP_EVENT_NOTE_EXPRESSION:
			{
			auto expression_event = (const clap_event_note_expression_t*) event;
			if (expression_event->expression_id == CLAP_NOTE_EXPRESSION_TUNING)
				synth->tuning_expression_changed(expression_event->value);
			}
			break;
		}
}



