#include "SubsoundWidget.h"
#include "SFZSound.h"
#include "Button.h"
#include "Label.h"

static const double spacing = 6.0;


SubsoundWidget::SubsoundWidget(CairoGUI* gui_in, SFZSound* sound_in, Rect rect_in)
	: Widget(gui_in, rect_in), sound(sound_in)
{
	minus_button = new Button(gui, "\u2B05");
	plus_button = new Button(gui, "\u27A1");
	name_label = new Label(gui, sound->subsound_name(sound->selected_subsound()));
	name_label->font_weight = CAIRO_FONT_WEIGHT_NORMAL;
	if (rect.width > 0)
		layout();
}

SubsoundWidget::~SubsoundWidget()
{
	delete minus_button;
	delete plus_button;
	delete name_label;
}


void SubsoundWidget::layout()
{
	minus_button->rect = { rect.x, rect.y, rect.height, rect.height };
	plus_button->rect = { rect.x + rect.width - rect.height, rect.y, rect.height, rect.height };
	name_label->rect = { rect.x + rect.height + spacing, rect.y, rect.width - 2 * (rect.height + spacing), rect.height };
}


void SubsoundWidget::update()
{
	name_label->label = sound->subsound_name(sound->selected_subsound());
}


void SubsoundWidget::paint()
{
	minus_button->paint();
	name_label->paint();
	plus_button->paint();
}

void SubsoundWidget::mouse_pressed(int x, int y)
{
	auto selected_subsound = sound->selected_subsound();
	if (minus_button->contains(x, y)) {
		if (selected_subsound > 0)
			tracking_widget = minus_button;
		}
	else if (plus_button->contains(x, y)) {
		if (selected_subsound < sound->num_subsounds() - 1)
			tracking_widget = plus_button;
		}
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



