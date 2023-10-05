#include "Button.h"
#include "CairoGUI.h"
#include <iostream>

static const double label_size = 0.6;
static const double corner_size = 8.0;


void Button::paint()
{
	auto cairo = gui->cairo();
	cairo_save(cairo);

	// Border/background.
	auto cur_corner_size = corner_size;
	if (cur_corner_size >= rect.height / 2)
		cur_corner_size = rect.height * 0.25;
	rounded_rect(rect, cur_corner_size);
	if (pressed && is_mouse_over)
		cairo_set_source_rgb(cairo, 0.8, 0.8, 0.8);
	else
		cairo_set_source_rgb(cairo, 1.0, 1.0, 1.0);
	cairo_fill_preserve(cairo);
	cairo_set_source_rgb(cairo, 0, 0, 0);
	if (rect.height <= 24)
		cairo_set_line_width(cairo, 1.0);
	cairo_stroke(cairo);

	// Label.
	cairo_select_font_face(cairo, (font ? font : gui->default_font()), CAIRO_FONT_SLANT_NORMAL, font_weight);
	cairo_set_font_size(cairo, rect.height * label_size);
	cairo_text_extents_t text_extents, ascent_extents;
	cairo_text_extents(cairo, label, &text_extents);
	cairo_text_extents(cairo, "M", &ascent_extents);
	cairo_font_extents_t font_extents;
	cairo_font_extents(cairo, &font_extents);
	cairo_move_to(
		cairo,
		rect.x + (rect.width - text_extents.width) / 2 - text_extents.x_bearing,
		rect.y + (rect.height + ascent_extents.height) / 2);
	cairo_set_source_rgba(cairo, 0, 0, 0, (enabled ? 1.0 : 0.4));
	cairo_show_text(cairo, label);

	cairo_restore(cairo);
}


void Button::mouse_pressed(int x, int y)
{
	if (!enabled)
		return;
	pressed = true;
	is_mouse_over = true;
}

bool Button::mouse_released(int x, int y)
{
	pressed = false;
	return enabled && contains(x, y);
}

void Button::mouse_moved(int x, int y)
{
	if (pressed)
		is_mouse_over = contains(x, y);
}




