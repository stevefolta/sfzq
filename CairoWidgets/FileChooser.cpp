#include "FileChooser.h"
#include "CairoGUI.h"
#include "FileList.h"
#include "Button.h"
#include <unistd.h>

static const double spacing = 6.0;
static const double up_button_size = 16.0;
static const double button_width = 80.0;
static const double button_height = 24.0;


FileChooser::FileChooser(CairoGUI* gui, Rect rect)
	: Widget(gui, rect)
{
	Rect temp_rect = {};
	file_list = new FileList(gui, temp_rect);
	up_button = new Button(gui, temp_rect, "\u2B06");
	Rect button_rect = { 0, 0, button_width, button_height };
	ok_button = new Button(gui, button_rect, "OK");
	cancel_button = new Button(gui, button_rect, "Cancel");
	layout();
}


FileChooser::~FileChooser()
{
	delete file_list;
	delete up_button;
	delete ok_button;
	delete cancel_button;
}


void FileChooser::set_path(std::string new_path)
{
	path = new_path;
	file_list->set_dir(new_path);
	up_button->enabled = (path != "/");
}

void FileChooser::go_to_cwd()
{
	set_path(cwd());
}

void FileChooser::set_file_filter(std::function<bool(const char*)> new_file_filter)
{
	file_list->file_filter = new_file_filter;
}

void FileChooser::set_font(const char* font, double size, cairo_font_weight_t weight)
{
	file_list->set_font(font, size, weight);
}

void FileChooser::set_path_font(const char* font, double size, cairo_font_weight_t weight)
{
	path_font = font;
	path_font_size = size;
	path_font_weight = weight;
}

void FileChooser::set_button_font(const char* font, cairo_font_weight_t weight)
{
	up_button->set_font(font, weight);
	ok_button->set_font(font, weight);
	cancel_button->set_font(font, weight);
}


void FileChooser::paint()
{
	// Default to the current working directory... but defer that until now, to
	// give the owner a change to set things up.
	if (path.empty())
		go_to_cwd();

	draw_path();
	file_list->paint();
	up_button->paint();
	if (ok_fn)
		ok_button->paint();
	if (cancel_fn)
		cancel_button->paint();
}


void FileChooser::mouse_pressed(int x, int y)
{
	if (file_list->contains(x, y))
		tracking_widget = file_list;
	else if (up_button->contains(x, y))
		tracking_widget = up_button;
	else if (ok_fn && ok_button->contains(x, y))
		tracking_widget = ok_button;
	else if (cancel_fn && cancel_button->contains(x, y))
		tracking_widget = cancel_button;

	if (tracking_widget) {
		tracking_widget->mouse_pressed(x, y);

		if (tracking_widget == file_list && file_list->was_double_clicked) {
			tracking_widget = nullptr;
			enter_selected_entry();
			}
		}
}

bool FileChooser::mouse_released(int x, int y)
{
	bool accepted = false;
	if (tracking_widget)
		accepted = tracking_widget->mouse_released(x, y);

	// Clear "tracking_widget", but do it before potentially calling ok_fn or
	// cancel_fn.  Those functions may want to delete "this", so we don't want to
	// access "this" after calling them.
	auto released_widget = tracking_widget;
	tracking_widget = nullptr;

	if (accepted) {
		if (released_widget == up_button)
			go_up();
		else if (released_widget == ok_button && ok_fn) {
			auto name = file_list->selection_name();
			if (!name.empty())
				ok_fn(path + "/" + name);
			}
		else if (released_widget == cancel_button && cancel_fn)
			cancel_fn();
		}

	return false;
}

void FileChooser::mouse_moved(int x, int y)
{
	if (tracking_widget && tracking_widget != file_list)
		tracking_widget->mouse_moved(x, y);
	file_list->mouse_moved(x, y);
}


void FileChooser::key_pressed(std::string_view key)
{
	if (key == "\n" || key == "\r")
		enter_selected_entry();
	else
		file_list->key_pressed(key);
}

void FileChooser::special_key_pressed(std::string_view special_key)
{
	file_list->special_key_pressed(special_key);
}


void FileChooser::scroll_down(int x, int y)
{
	if (file_list->contains(x, y))
		file_list->scroll_down(x, y);
}

void FileChooser::scroll_up(int x, int y)
{
	if (file_list->contains(x, y))
		file_list->scroll_up(x, y);
}


void FileChooser::resize_to(int width, int height)
{
	rect.width = width;
	rect.height = height;
	layout();
}


void FileChooser::layout()
{
	up_button->rect = {
		rect.x + rect.width - up_button_size, rect.y,
		up_button_size, up_button_size };

	file_list->rect.x = rect.x;
	file_list->rect.y = rect.y + up_button_size + spacing;
	double file_list_height = rect.height - up_button_size - spacing;
	if (ok_fn || cancel_fn)
		file_list_height -= button_height + spacing;
	file_list->resize_to(rect.width, file_list_height);

	ok_button->rect.x = rect.x + rect.width - button_width;
	ok_button->rect.y = rect.y + rect.height - button_height;
	cancel_button->rect.x = ok_button->rect.x;
	if (ok_fn)
		cancel_button->rect.x -= spacing + button_width;
	cancel_button->rect.y = ok_button->rect.y;
}


void FileChooser::draw_path()
{
	auto cairo = gui->cairo();
	cairo_save(cairo);

	// Clip.
	auto path_rect = rect;
	path_rect.width -= up_button_size + spacing;
	path_rect.height = rect.x + up_button_size + spacing;
	cairo_rectangle(cairo, path_rect.x, path_rect.y, path_rect.width, path_rect.height);
	cairo_clip(cairo);

	// Draw.
	cairo_select_font_face(cairo, (path_font ? path_font : gui->default_font()), CAIRO_FONT_SLANT_NORMAL, path_font_weight);
	cairo_set_font_size(cairo, path_font_size);
	cairo_font_extents_t font_extents;
	cairo_font_extents(cairo, &font_extents);
	cairo_move_to(cairo, rect.x, rect.y + font_extents.ascent);
	cairo_set_source_rgb(cairo, 0, 0, 0);
	cairo_show_text(cairo, path.c_str());

	cairo_restore(cairo);
}


std::string FileChooser::cwd()
{
	std::string wd;
	size_t buffer_size = 256;
	while (wd.empty()) {
		char* buffer = (char*) malloc(buffer_size);
		if (getcwd(buffer, buffer_size))
			wd = buffer;
		free(buffer);
		buffer_size *= 2;
		}
	return wd;
}


void FileChooser::enter_selected_entry()
{
	auto name = file_list->selection_name();
	if (name.empty())
		return;
	std::string entry_path = "/" + name;
	if (path != "/")
		entry_path = path + entry_path;

	if (file_list->selection_is_dir())
		set_path(entry_path);

	else {
		if (ok_fn)
			ok_fn(entry_path);
		}
}


void FileChooser::go_up()
{
	auto last_slash = path.find_last_of('/');
	auto new_path = path.substr(0, last_slash);
	if (new_path.empty())
		new_path = "/";
	set_path(new_path);
}



