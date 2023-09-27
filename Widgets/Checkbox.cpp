#include "Checkbox.h"
#include "CairoGUI.h"


void Checkbox::paint()
{
	auto cairo = gui->cairo();
	cairo_save(cairo);

	// Text metrics.
	cairo_select_font_face(cairo, (font ? font : gui->default_font()), CAIRO_FONT_SLANT_NORMAL, font_weight);
	cairo_set_font_size(cairo, rect.height);
	cairo_font_extents_t font_extents;
	cairo_font_extents(cairo, &font_extents);
	cairo_text_extents_t em_box;
	cairo_text_extents(cairo, "M", &em_box);

	// Box.
	double box_size = em_box.height;
	double baseline = rect.y + font_extents.ascent;
	cairo_rectangle(cairo, rect.x, baseline - box_size, box_size, box_size);
	cairo_set_line_width(cairo, pressed && hovering && enabled ? 2.0 : 1.0);
	cairo_set_source_rgb(cairo, box_color.red, box_color.green, box_color.blue);
	cairo_stroke(cairo);

	// Check.
	if (checked) {
		cairo_move_to(cairo, rect.x, baseline - box_size / 2);
		cairo_line_to(cairo, rect.x + 5 * box_size / 12, baseline - box_size / 3);
		cairo_line_to(cairo, rect.x + box_size, rect.y);
		cairo_line_to(cairo, rect.x + box_size / 2, baseline);
		cairo_close_path(cairo);
		cairo_set_source_rgb(cairo, box_color.red, box_color.green, box_color.blue);
		cairo_fill(cairo);
		}

	// Text.
	cairo_move_to(cairo, rect.x + box_size + em_box.width / 2, baseline);
	cairo_set_source_rgb(cairo, text_color.red, text_color.green, text_color.blue);
	cairo_show_text(cairo, text.c_str());

	cairo_restore(cairo);
}

double Checkbox::drawn_width()
{
	auto cairo = gui->cairo();
	if (cairo == nullptr)
		return 0;
	cairo_save(cairo);

	cairo_select_font_face(cairo, (font ? font : gui->default_font()), CAIRO_FONT_SLANT_NORMAL, font_weight);
	cairo_set_font_size(cairo, rect.height);
	cairo_text_extents_t em_box;
	cairo_text_extents(cairo, "M", &em_box);
	cairo_text_extents_t text_box;
	cairo_text_extents(cairo, text.c_str(), &text_box);

	double box_size = em_box.height;
	cairo_restore(cairo);
	return box_size + em_box.width / 2 + text_box.x_advance;
}


void Checkbox::mouse_pressed(int x, int y)
{
	pressed = hovering = true;
}

bool Checkbox::mouse_released(int x, int y)
{
	pressed = hovering = false;
	bool ok = enabled && contains(x, y);
	if (ok)
		checked = !checked;
	return ok;
}

void Checkbox::mouse_moved(int x, int y)
{
	if (pressed)
		hovering = contains(x, y);
}



