#include "TuningFile.h"
#include "SFZQPlugin.h"
#include "Checkbox.h"
#include "Label.h"


static const Color grey_color = { 0.5, 0.5, 0.5 };
static const Color black_color = { 0, 0, 0 };


void TuningFile::init(CairoGUI* gui_in, SFZQPlugin* plugin_in, std::string initial_label, std::string main_label_in)
{
	gui = gui_in;
	plugin = plugin_in;
	main_label = main_label_in;

	checkbox = new Checkbox(gui, initial_label);
	checkbox->text_color = grey_color;
	label = new Label(gui, "");
	label->font_weight = CAIRO_FONT_WEIGHT_NORMAL;
}


TuningFile::~TuningFile()
{
	delete checkbox;
	delete label;
}


void TuningFile::paint()
{
	checkbox->paint();
	label->paint();
}


void TuningFile::mouse_pressed(int x, int y)
{
	if (checkbox->contains(x, y))
		tracking_widget = checkbox;
	else if (label->contains(x, y))
		plugin->open_tuning_file_chooser(this);
	if (tracking_widget)
		tracking_widget->mouse_pressed(x, y);
}

bool TuningFile::mouse_released(int x, int y)
{
	if (tracking_widget == nullptr)
		return false;
	if (!tracking_widget->mouse_released(x, y)) {
		tracking_widget = nullptr;
		return false;
		}

	if (tracking_widget == checkbox) {
		if (checkbox->checked) {
			if (path.empty())
				plugin->open_tuning_file_chooser(this);
			else
				enabled = true;
			}
		else
			enabled = false;
		plugin->tuning_changed();
		update();
		}

	tracking_widget = nullptr;
	return true;
}

void TuningFile::mouse_moved(int x, int y)
{
	if (tracking_widget)
		tracking_widget->mouse_moved(x, y);
}


void TuningFile::layout()
{
	checkbox->rect = rect;
	checkbox->rect.width = checkbox->drawn_width();
	label->rect = {
		checkbox->rect.x + checkbox->rect.width, rect.y,
		rect.width, rect.height,
		};
	label->rect.width = label->drawn_width();

	// Update our own rect's width.
	rect.width = checkbox->rect.width + label->rect.width;
}


void TuningFile::enable()
{
	enabled = true;
	update();
}

void TuningFile::update()
{
	checkbox->checked = enabled;
	label->color = enabled ? black_color : grey_color;
}

void TuningFile::set_path(std::string new_path)
{
	path = new_path;
	if (!path.empty()) {
		checkbox->text = main_label;
		checkbox->text_color = black_color;
		label->label = path.substr(path.find_last_of('/') + 1);
		}
}




