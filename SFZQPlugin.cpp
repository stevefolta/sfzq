#include "SFZQPlugin.h"
#include "SFZSynth.h"
#include "SFZSound.h"
#include "SF2Sound.h"
#include "Button.h"
#include "FileChooser.h"
#include "Label.h"
#include "ProgressBar.h"
#include "SubsoundWidget.h"
#include "TextBox.h"
#include "KeyboardWidget.h"
#include "CLAPPosixFDExtension.h"
#include "CLAPCairoGUIExtension.h"
#include "CLAPAudioPortsExtension.h"
#include "CLAPNotePortsExtension.h"
#include "CLAPParamsExtension.h"
#include "CLAPStateExtension.h"
#include "CLAPStream.h"
#include "CLAPOutBuffer.h"
#include "Messages.h"
#include "Settings.h"
#include <thread>
#include <string_view>
#include <iostream>

static const double filename_label_height = 24.0;
static const double progress_bar_height = 20.0;
static const double subsound_widget_height = 16.0;
static const double voices_used_label_height = 12.0;
static const double keyboard_height = 40.0;
static const double margin = 10.0;
static const double spacing = 6.0;
static const double file_chooser_alpha = 0.9;
static const int num_voices_update_frames = 22050;


SFZQPlugin::SFZQPlugin(const clap_plugin_descriptor_t* descriptor, const clap_host_t* host)
	: CLAPPlugin(descriptor, host),
	main_to_audio_queue(20), audio_to_main_queue(20), load_to_main_queue(100),
	cairo_gui(this)
{
	posix_fd_extension = new CLAPPosixFDExtension(this);
	cairo_gui_extension = new CLAPCairoGUIExtension(this);
	audio_ports_extension = new CLAPAudioPortsExtension(this);
	note_ports_extension = new CLAPNotePortsExtension(this);
	params_extension = new CLAPParamsExtension();
	state_extension = new CLAPStateExtension(this);

	settings.read_settings_files();

	synth = new SFZSynth(settings.num_voices);

	// GUI.
	filename_label = new Label(&cairo_gui, "Click here to open SFZ file...");
	filename_label->color = { 0.5, 0.5, 0.5 };
	error_box = new TextBox(&cairo_gui);
	error_box->text = settings.errors;
	keyboard = new KeyboardWidget(&cairo_gui);
	if (settings.show_voices_used) {
		voices_used_label = new Label(&cairo_gui, "Voices used:");
		voices_used_label->font_weight = CAIRO_FONT_WEIGHT_NORMAL;
		}
	layout();
}

SFZQPlugin::~SFZQPlugin()
{
	delete filename_label;
	delete progress_bar;
	delete subsound_widget;
	delete error_box;
	delete voices_used_label;
	delete keyboard;
	delete file_chooser;

	if (load_samples_thread.joinable())
		load_samples_thread.join();

	delete synth;
	delete loading_sound;

	delete cairo_gui_extension;
	delete posix_fd_extension;
	delete audio_ports_extension;
	delete note_ports_extension;
	delete params_extension;
	delete state_extension;
}


static const std::vector<clap_note_port_info_t> note_in_ports = {
	{
		.id = 0,
		.supported_dialects = CLAP_NOTE_DIALECT_CLAP,
		.preferred_dialect = CLAP_NOTE_DIALECT_CLAP,
		.name = "in",
		},
	};

static const std::vector<clap_audio_port_info_t> audio_out_ports = {
	{
		.id = 0,
		.name = "out",
		.flags =
			CLAP_AUDIO_PORT_IS_MAIN
#ifndef SUPPORT_32_BIT_ONLY
			| CLAP_AUDIO_PORT_SUPPORTS_64BITS
#endif
			,
		.channel_count = 2,
		.port_type = CLAP_PORT_STEREO,
		.in_place_pair = CLAP_INVALID_ID,
		},
	};

uint32_t SFZQPlugin::num_note_ports(bool is_input)
{
	return is_input ? note_in_ports.size() : 0;
}

bool SFZQPlugin::get_note_port_info(uint32_t index, bool is_input, clap_note_port_info_t* info_out)
{
	if (is_input && index < note_in_ports.size()) {
		*info_out = note_in_ports[index];
		return true;
		}
	return false;
}

uint32_t SFZQPlugin::num_audio_ports(bool is_input)
{
	return (!is_input ? audio_out_ports.size() : 0);
}

bool SFZQPlugin::get_audio_port_info(uint32_t index, bool is_input, clap_audio_port_info_t* info_out)
{
	if (!is_input && index < audio_out_ports.size()) {
		*info_out = audio_out_ports[index];
		return true;
		}
	return false;
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

	// Set up to send note-end events back to the host.
	synth->set_note_off_fn([&](int note, int channel, int note_id) {
		clap_event_note_t event = {};
		event.header.size = sizeof(event);
		event.header.time = 0;
		event.header.space_id = CLAP_CORE_EVENT_SPACE_ID;
		event.header.type = CLAP_EVENT_NOTE_END;
		event.header.flags = 0;
		event.key = note;
		event.note_id = note_id;
		event.channel = channel;
		event.port_index = 0;
		process->out_events->try_push(process->out_events, &event.header);
		});

	// Handle messages from the main thread.
	auto message = main_to_audio_queue.pop_front();
	if (message.id == UseSound) {
		auto old_sound = synth->set_sound((SFZSound*) message.param);
		if (old_sound) {
			audio_to_main_queue.send(DoneWithSound, old_sound);
			if (host)
				host->request_callback(host);
			}
		}
	else if (message.id == UseSubsound) {
		synth->use_subsound(message.num);
		audio_to_main_queue.send(SubsoundChanged, synth->selected_subsound());
		if (host)
			host->request_callback(host);
		}

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

	// Send info back to the main thread.
	if (settings.show_voices_used) {
		frames_until_num_voices_update -= num_frames;
		if (frames_until_num_voices_update <= 0) {
			audio_to_main_queue.send(VoicesUsed, synth->num_voices_used());
			if (host)
				host->request_callback(host);
			frames_until_num_voices_update = num_voices_update_frames;
			}
		}

	synth->set_note_off_fn({});
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
	gui_width = width;
	gui_height = height;
	layout();
	return true;
}


void SFZQPlugin::paint_gui()
{
	auto cairo = cairo_gui_extension->cairo;
	cairo_push_group(cairo);

	// Background.
	cairo_set_source_rgb(cairo, 1.0, 1.0, 1.0);
	cairo_paint(cairo);

	filename_label->paint();
	if (progress_bar)
		progress_bar->paint();
	if (subsound_widget)
		subsound_widget->paint();
	if (voices_used_label)
		voices_used_label->paint();
	error_box->paint();
	keyboard->paint();

	if (file_chooser) {
		cairo_push_group(cairo);
		cairo_set_source_rgb(cairo, 1.0, 1.0, 1.0);
		cairo_paint(cairo);
		file_chooser->paint();
		cairo_pop_group_to_source(cairo);
		cairo_paint_with_alpha(cairo, file_chooser_alpha);
		}

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

	if (file_chooser && file_chooser->contains(x, y))
		tracking_widget = file_chooser;
	else if (filename_label->contains(x, y))
		open_file_chooser();
	else if (subsound_widget && subsound_widget->contains(x, y))
		tracking_widget = subsound_widget;
	if (tracking_widget)
		tracking_widget->mouse_pressed(x, y);
	cairo_gui_extension->refresh();
}

void SFZQPlugin::mouse_released(int32_t x, int32_t y, int button)
{
	if (button != 1)
		return;

	if (tracking_widget && tracking_widget->mouse_released(x, y)) {
		// If we ever have any buttons in the main UI, handle them here.
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
	// Get messages from audio thread.
	int newest_voices_used = -1;
	while (true) {
		auto message = audio_to_main_queue.pop_front();
		if (message.id < 0)
			break;
		switch (message.id) {
			case DoneWithSound:
				delete (SFZSound*) message.param;
				break;
			case SubsoundChanged:
				if (subsound_widget) {
					subsound_widget->update();
					refresh_requested = true;
					}
				sound_subsound = message.num;
				break;
			case VoicesUsed:
				newest_voices_used = message.num;
				break;
			}
		}
	if (newest_voices_used >= 0 && voices_used_label) {
		voices_used_label->label = "Voices used: " + std::to_string(newest_voices_used);
		refresh_requested = true;
		}

	// Get messages from load thread.
	while (true) {
		auto message = load_to_main_queue.pop_front();
		if (message.id < 0)
			break;
		if (message.id == SampleLoadComplete) {
			sound_path = loading_sound->get_path();
			sound_subsound = loading_sound->selected_subsound();
			delete progress_bar;
			progress_bar = nullptr;
			delete subsound_widget;
			subsound_widget = nullptr;
			if (loading_sound->num_subsounds() > 1) {
				subsound_widget = new SubsoundWidget(&cairo_gui, loading_sound);
				subsound_widget->select_subsound_fn = [&](int which_subsound) {
					main_to_audio_queue.send(UseSubsound, which_subsound);
					state_extension->host_mark_dirty();
					};
				}
			error_box->text = loading_sound->get_errors_string();
			keyboard->use_sound(loading_sound);
			layout();
			main_to_audio_queue.send(UseSound, loading_sound);
			loading_sound = nullptr;
			refresh_requested = true;
			if (initial_load)
				initial_load = false;
			else
				state_extension->host_mark_dirty();
			}
		}

	bool need_refresh = refresh_requested.exchange(false);
	if (need_refresh)
		cairo_gui_extension->refresh();
}


bool SFZQPlugin::save_state(const clap_ostream_t* clap_stream)
{
	CLAPOutStream stream(clap_stream);
	stream.write_uint32(1); 	// Version.
	stream.write_string(sound_path);
	stream.write_uint32(sound_subsound);
	return stream.ok;
}

bool SFZQPlugin::load_state(const clap_istream_t* clap_stream)
{
	CLAPInStream stream(clap_stream);
	int version = stream.read_uint32();
	if (!stream.ok || version != 1)
		return false;
	auto path = stream.read_string();
	if (!stream.ok)
		return false;
	int subsound = 0;
	int read_subsound = stream.read_uint32();
	if (stream.ok)
		subsound = read_subsound;
	if (!path.empty()) {
		initial_load = true;
		load_sound(path, subsound);
		}
	return true;
}


void SFZQPlugin::process_event(const clap_event_header_t* event)
{
	switch (event->type) {
		case CLAP_EVENT_NOTE_ON:
			{
			auto note_event = (const clap_event_note_t*) event;
			synth->note_on(note_event->key, note_event->velocity, note_event->channel, note_event->note_id);
			}
			break;
		case CLAP_EVENT_NOTE_OFF:
		case CLAP_EVENT_NOTE_CHOKE:
			{
			auto note_event = (const clap_event_note_t*) event;
			synth->note_off(
				note_event->key, note_event->velocity, note_event->channel, note_event->note_id,
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


void SFZQPlugin::layout()
{
	auto contents_width = gui_width - 2 * margin;
	filename_label->rect = { margin, margin, contents_width, filename_label_height };
	double top = margin + filename_label_height + spacing;
	if (progress_bar) {
		progress_bar->rect = {
			margin, top,
			contents_width, progress_bar_height };
		top += progress_bar_height + spacing;
		}
	if (subsound_widget) {
		subsound_widget->rect = { margin, top, contents_width, subsound_widget_height };
		subsound_widget->layout();
		top += subsound_widget_height + spacing;
		}
	if (voices_used_label) {
		voices_used_label->rect = { margin, top, contents_width, voices_used_label_height };
		top += voices_used_label_height + spacing;
		}
	double keyboard_top = gui_height - margin - keyboard_height;
	keyboard->rect = { margin, keyboard_top, contents_width, keyboard_height };
	error_box->rect = { margin, top, contents_width, keyboard_top - top };
	if (file_chooser) {
		file_chooser->rect = { margin, margin, 0, 0 };
		file_chooser->resize_to(contents_width, gui_height - 2 * margin);
		}
}


void SFZQPlugin::open_file_chooser()
{
	if (file_chooser)
		return;

	file_chooser = new FileChooser(&cairo_gui, {});
	if (!settings.samples_directory.empty())
		file_chooser->set_path(settings.samples_directory);
	file_chooser->set_file_filter([](const char* filename_in) {
		std::string_view filename(filename_in);
		auto dot_pos = filename.rfind('.');
		if (dot_pos == std::string_view::npos)
			return false;
		auto extension = filename.substr(dot_pos + 1);
		return extension == "sfz" || extension == "SFZ";
		});
	file_chooser->set_ok_fn([&](std::string path) {
		delete file_chooser;
		file_chooser = nullptr;
		tracking_widget = nullptr;
		load_sound(path);
		});
	file_chooser->set_cancel_fn([&]() {
		delete file_chooser;
		file_chooser = nullptr;
		tracking_widget = nullptr;
		});
	layout();
}


void SFZQPlugin::load_sound(std::string path, int subsound)
{
	filename_label->label = path.substr(path.find_last_of('/') + 1);
	filename_label->color = { 0.0, 0.0, 0.0 };

	auto extension = path.substr(path.find_last_of('.') + 1);
	if (extension == "sf2" || extension == "SF2")
		loading_sound = new SF2Sound(path);
	else
		loading_sound = new SFZSound(path);
	loading_sound->load_regions();
	loading_sound->use_subsound(subsound);

	progress_bar = new ProgressBar(&cairo_gui);
	progress_bar->max = 1.0;
	layout();

	if (load_samples_thread.joinable())
		load_samples_thread.join();
	load_samples_thread = std::thread([&]() { load_samples(); });
}


void SFZQPlugin::load_samples()
{
	loading_sound->load_samples([&](double progress) {
		if (progress_bar == nullptr)
			return;
		progress_bar->current = progress;
		refresh_requested = true;
		if (host)
			host->request_callback(host);
		});

	// Let the main thread know we're done.
	load_to_main_queue.send(SampleLoadComplete);
	if (host)
		host->request_callback(host);
}



