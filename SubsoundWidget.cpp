#include "SubsoundWidget.h"
#include "SFZSound.h"
#include "Button.h"
#include "Label.h"

static const double spacing = 6.0;
static const int more_increment = 10;


SubsoundWidget::SubsoundWidget(CairoGUI* gui_in, SFZSound* sound_in, Rect rect_in)
	: Widget(gui_in, rect_in), sound(sound_in)
{
	minus_button = new Button(gui, "\u25C0");
	plus_button = new Button(gui, "\u25B6");
	minus_button->label_size = plus_button->label_size = 0.65;
	name_label = new Label(gui, "");
	name_label->font_weight = CAIRO_FONT_WEIGHT_NORMAL;
	if (sound->num_subsounds() > more_increment) {
		// Would use REW/FF (\u23E4 and \u23E9), but they seem rarer.
		minus_more_button = new Button(gui, "\u25C0\u25C0");
		plus_more_button = new Button(gui, "\u25B6\u25B6");
		minus_more_button->label_size = plus_more_button->label_size = 0.65;
		}
	update();
	if (rect.width > 0)
		layout();
}

SubsoundWidget::~SubsoundWidget()
{
	delete minus_button;
	delete plus_button;
	delete minus_more_button;
	delete plus_more_button;
	delete name_label;
}


void SubsoundWidget::layout()
{
	double button_size = rect.height;
	double more_button_width = button_size * 1.4;
	double left = rect.x;
	if (minus_more_button) {
		minus_more_button->rect = { left, rect.y, more_button_width, button_size };
		left += more_button_width + spacing;
		}
	minus_button->rect = { left, rect.y, button_size, button_size };
	left += button_size + spacing;
	double right = rect.x + rect.width;
	if (plus_more_button) {
		plus_more_button->rect = { right - more_button_width, rect.y, more_button_width, button_size };
		right -= more_button_width + spacing;
		}
	plus_button->rect = { right - button_size, rect.y, button_size, button_size };
	right -= button_size + spacing;
	name_label->rect = { left, rect.y, right - left, rect.height };
}


void SubsoundWidget::update()
{
	auto selected_subsound = sound->selected_subsound();
	int last_subsound = sound->num_subsounds() - 1;
	name_label->label = sound->subsound_name(selected_subsound);
	if (minus_more_button)
		minus_more_button->enabled = selected_subsound > 0;
	minus_button->enabled = selected_subsound > 0;
	plus_button->enabled = selected_subsound < last_subsound;
	if (plus_more_button)
		plus_more_button->enabled = selected_subsound < last_subsound;
}


void SubsoundWidget::paint()
{
	if (minus_more_button)
		minus_more_button->paint();
	minus_button->paint();
	name_label->paint();
	plus_button->paint();
	if (plus_more_button)
		plus_more_button->paint();
}

void SubsoundWidget::mouse_pressed(int x, int y)
{
	if (minus_button->contains(x, y))
		tracking_widget = minus_button;
	else if (plus_button->contains(x, y))
		tracking_widget = plus_button;
	else if (minus_more_button->contains(x, y))
		tracking_widget = minus_more_button;
	else if (plus_more_button->contains(x, y))
		tracking_widget = plus_more_button;
	if (tracking_widget)
		tracking_widget->mouse_pressed(x, y);
}

bool SubsoundWidget::mouse_released(int x, int y)
{
	bool acted = false;
	if (tracking_widget && tracking_widget->mouse_released(x, y)) {
		auto selected_subsound = sound->selected_subsound();
		acted = true;
		if (tracking_widget == minus_button)
			select_subsound_fn(selected_subsound - 1);
		else if (tracking_widget == plus_button)
			select_subsound_fn(selected_subsound + 1);
		else if (tracking_widget == minus_more_button) {
			int new_subsound = selected_subsound - more_increment;
			if (new_subsound < 0)
				new_subsound = 0;
			select_subsound_fn(new_subsound);
			}
		else if (tracking_widget == plus_more_button) {
			int new_subsound = selected_subsound + more_increment;
			auto num_subsounds = sound->num_subsounds();
			if (new_subsound >= num_subsounds)
				new_subsound = num_subsounds - 1;
			select_subsound_fn(new_subsound);
			}
		else
			acted = false;
		}
	tracking_widget = nullptr;
	return acted;
}

void SubsoundWidget::mouse_moved(int x, int y)
{
	if (tracking_widget)
		tracking_widget->mouse_moved(x, y);
}



