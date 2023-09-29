#include "FileList.h"
#include "CairoGUI.h"
#include "Scrollbar.h"
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>
#include <algorithm>
#include <iostream>

static const double scrollbar_width = 6.0;
static const double scrollbar_inset = 10.0;
static const double text_inset = 10.0;
static const Color dir_color = { 0.0, 0.0, 0.5 };
static const Color selection_color = { 0.9, 0.9, 0.95 };
static const double wheel_scroll_amount = 30.0;
static const double double_click_time = 1.0;


FileList::FileList(CairoGUI* gui, Rect rect)
	: Widget(gui, rect)
{
	scrollbar = new Scrollbar(gui, scrollbar_rect());
	last_click_time.clear();
}


FileList::~FileList()
{
	delete scrollbar;
}


void FileList::set_dir(std::string path)
{
	entries.clear();
	search_string.clear();
	DIR* dir = opendir(path.c_str());
	if (dir == NULL) {
		update_scrollbar();
		return;
		}

	struct dirent* entry = nullptr;
	while ((entry = readdir(dir))) {
		if (entry->d_name[0] == '.')
			continue;
		bool is_dir = false;
#ifdef _DIRENT_HAVE_D_TYPE
		is_dir = entry->d_type == DT_DIR;
		bool know_type = entry->d_type != DT_UNKNOWN;
#else
		const bool know_type = false;
#endif
		if (!know_type) {
			struct stat stat_buf;
			if (stat((path + "/" + entry->d_name).c_str(), &stat_buf) == 0)
				is_dir = ((stat_buf.st_mode & S_IFMT) == S_IFDIR);
			}
		if (is_dir || !file_filter || file_filter(entry->d_name))
			entries.push_back({ entry->d_name, is_dir });
		}

	closedir(dir);

	std::sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b) { return a.name < b.name; });

	selected_index = -1;
	scrollbar->value = 0.0;
	update_scrollbar();
	last_click_time.clear();
}


void FileList::set_font(const char* font, double size, cairo_font_weight_t weight)
{
	font = font;
	font_size = size;
	font_weight = weight;
	update_scrollbar();
}


void FileList::paint()
{
	if (needs_scrollbar_update)
		update_scrollbar();

	// Clip.
	auto cairo = gui->cairo();
	cairo_save(cairo);
	cairo_rectangle(cairo, rect.x, rect.y, rect.width, rect.height);
	cairo_clip(cairo);

	// Background.
	cairo_rectangle(cairo, rect.x, rect.y, rect.width, rect.height);
	cairo_set_source_rgb(cairo, 1.0, 1.0, 1.0);
	cairo_fill(cairo);

	// Get font info.
	cairo_select_font_face(cairo, (font ? font : gui->default_font()), CAIRO_FONT_SLANT_NORMAL, font_weight);
	cairo_set_font_size(cairo, font_size);
	cairo_font_extents_t font_extents;
	cairo_font_extents(cairo, &font_extents);
	double line_height = font_extents.height;
	double ascent = font_extents.ascent;

	// Draw the list.
	int index = scrollbar->value / line_height;
	int end = index + (int) (rect.height / line_height) + 2;
	double cell_top = index * line_height - scrollbar->value;
	double baseline = rect.y + cell_top + ascent;
	for (; index < end && index < (int) entries.size(); ++index) {
		// Selection.
		if (index == selected_index) {
			cairo_rectangle(cairo, rect.x, baseline - ascent, rect.width, line_height);
			cairo_set_source_rgb(cairo, selection_color.red, selection_color.green, selection_color.blue);
			cairo_fill(cairo);
			}

		// Text.
		if (entries[index].is_dir)
			cairo_set_source_rgb(cairo, dir_color.red, dir_color.green, dir_color.blue);
		else
			cairo_set_source_rgb(cairo, 0, 0, 0);
		cairo_move_to(cairo, rect.x + text_inset, baseline);
		cairo_show_text(cairo, entries[index].name.c_str());

		baseline += line_height;
		}
	cairo_set_source_rgb(cairo, 0, 0, 0);

	scrollbar->paint();

	// Border.
	cairo_rectangle(cairo, rect.x, rect.y, rect.width, rect.height);
	cairo_set_source_rgb(cairo, 0, 0, 0);
	cairo_set_line_width(cairo, 1.0);
	cairo_stroke(cairo);

	cairo_restore(cairo);
}


void FileList::mouse_pressed(int x, int y)
{
	if (scrollbar->contains(x, y)) {
		scrollbar->mouse_pressed(x, y);
		tracking_widget = scrollbar;
		return;
		}

	auto last_selected_index = selected_index;
	selected_index = (y - rect.y + scrollbar->value) / get_line_height();
	if (selected_index >= (int) entries.size())
		selected_index = -1;

	TimeSeconds now = TimeSeconds::now();
	was_double_clicked =
		selected_index == last_selected_index && selected_index >= 0 &&
		(now - last_click_time).as_double() < double_click_time;
	last_click_time = now;
}

bool FileList::mouse_released(int x, int y)
{
	if (tracking_widget)
		tracking_widget->mouse_released(x, y);
	tracking_widget = nullptr;
	return true;
}

void FileList::mouse_moved(int x, int y)
{
	scrollbar->mouse_moved(x, y);
}


void FileList::scroll_down(int x, int y)
{
	scrollbar->value += wheel_scroll_amount;
	double max_value = scrollbar->max * (1 - scrollbar->percentage);
	if (scrollbar->value > max_value)
		scrollbar->value = max_value;
	gui->refresh();
}

void FileList::scroll_up(int x, int y) 
{
	scrollbar->value -= wheel_scroll_amount;
	if (scrollbar->value < 0.0)
		scrollbar->value = 0.0;
	gui->refresh();
}


void FileList::key_pressed(std::string_view key)
{
	if (entries.empty())
		return;

	// Update the search_string.
	if (key == "\b" || key == "\x7F") {
		if (!search_string.empty()) {
			// Remove last UTF-8 character.
			while ((search_string[search_string.size() - 1] & 0xC0) == 0x80)
				search_string = search_string.substr(0, search_string.size() - 1);
			search_string = search_string.substr(0, search_string.size() - 1);
			if (search_string.empty())
				return;
			}
		}
	else if (key < " ") {
		// Ignore control characters.
		return;
		}
	else
		search_string += key;

	// Search for the last entry less than the string.
	int left = 0;
	int right = entries.size();
	while (left < right) {
		int middle = (left + right) / 2;
		if (entries[middle].name < search_string)
			left = middle + 1;
		else
			right = middle;
		}
	selected_index = left;
	show_selected_item();
}

void FileList::special_key_pressed(std::string_view special_key)
{
	if (special_key == "<ArrowDown>") {
		if (selected_index < (int) entries.size() - 1)
			selected_index += 1;
		show_selected_item();
		}
	else if (special_key == "<ArrowUp>") {
		if (selected_index > 0)
			selected_index -= 1;
		show_selected_item();
		}
	else if (special_key == "<PageDown>") {
		selected_index += num_items_shown();
		if (selected_index >= (int) entries.size())
			selected_index = entries.size() - 1;
		show_selected_item();
		}
	else if (special_key == "<PageUp>") {
		selected_index -= num_items_shown();
		if (selected_index < 0)
			selected_index = 0;
		show_selected_item();
		}
	else if (special_key == "<Home>") {
		selected_index = 0;
		show_selected_item();
		}
	else if (special_key == "<End>") {
		selected_index = entries.size() - 1;
		show_selected_item();
		}
}


void FileList::resize_to(int width, int height)
{
	rect.width = width;
	rect.height = height;
	scrollbar->rect = scrollbar_rect();
	update_scrollbar();
}


std::string FileList::selection_name()
{
	if (selected_index < 0 || selected_index >= (int) entries.size())
		return "";
	return entries[selected_index].name;
}


bool FileList::selection_is_dir()
{
	if (selected_index < 0 || selected_index >= (int) entries.size())
		return false;
	return entries[selected_index].is_dir;
}



Rect FileList::scrollbar_rect()
{
	return {
		rect.x + rect.width - scrollbar_inset - scrollbar_width,
		rect.y + scrollbar_inset,
		scrollbar_width,
		rect.height - 2 * scrollbar_inset,
		};
}


void FileList::update_scrollbar()
{
	if (gui->cairo() == nullptr) {
		// Not ready yet; we'll defer this until it's possible.
		needs_scrollbar_update = true;
		return;
		}

	double line_height = get_line_height();
	scrollbar->max = entries.size() * line_height;
	scrollbar->percentage = rect.height / scrollbar->max;
	if (scrollbar->percentage > 1.0)
		scrollbar->percentage = 1.0;

	needs_scrollbar_update = false;
}


double FileList::get_line_height()
{
	auto cairo = gui->cairo();
	cairo_select_font_face(cairo, (font ? font : gui->default_font()), CAIRO_FONT_SLANT_NORMAL, font_weight);
	cairo_set_font_size(cairo, font_size);
	cairo_font_extents_t font_extents;
	cairo_font_extents(cairo, &font_extents);
	return font_extents.height;
}


int FileList::num_items_shown()
{
	return rect.height / get_line_height();
}


void FileList::show_selected_item()
{
	// Is it already (fully) visible?
	double line_height = get_line_height();
	int first_full_index = ceil(scrollbar->value / line_height);
	int last_full_index = floor((scrollbar->value + rect.height) / line_height) - 1;
	if (selected_index >= first_full_index && selected_index <= last_full_index)
		return;
	if (selected_index < 0)
		return;

	scrollbar->value = (selected_index + 1) * line_height - rect.height / 2;
	if (scrollbar->value < 0)
		scrollbar->value = 0;
	else if (scrollbar->value > scrollbar->max)
		scrollbar->value = scrollbar->max;
}



