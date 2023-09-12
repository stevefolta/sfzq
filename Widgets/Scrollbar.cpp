#include "Scrollbar.h"
#include "CairoGUI.h"
#include <iostream>

static const double narrow_corner = 4.0;
static const double wide_corner = 8.0;


void Scrollbar::paint()
{
	auto cairo = gui->cairo();
	cairo_save(cairo);

	// Background.
	rounded_rect(rect, narrow_corner, wide_corner);
	cairo_set_source_rgba(cairo, 0.5, 0.5, 0.5, 0.5);
	cairo_fill(cairo);

	// Thumb.
	if (max > 0.0) {
		auto thumb_rect = rect;
		thumb_rect.y += (value / max) * rect.height;
		thumb_rect.height = percentage * rect.height;
		if (thumb_rect.height < thumb_rect.width * 1.4)
			thumb_rect.height = thumb_rect.width * 1.4;
		rounded_rect(thumb_rect, narrow_corner, wide_corner);
		cairo_set_source_rgba(cairo, 0.5, 0.5, 0.5, 0.5);
		cairo_fill(cairo);
		}

	cairo_restore(cairo);
}


void Scrollbar::mouse_pressed(int x, int y)
{
	scrolling = true;
	start_value = value;
	start_y = y;
}

bool Scrollbar::mouse_released(int x, int y)
{
	scrolling = false;
	return contains(x, y);
}

void Scrollbar::mouse_moved(int x, int y)
{
	if (!scrolling)
		return;

	int delta = y - start_y;
	value = start_value + delta * max / rect.height;
	double max_value = max * (1 - percentage);
	if (value < 0.0)
		value = 0.0;
	else if (value > max_value)
		value = max_value;
	if (value_changed)
		value_changed(value);
}



