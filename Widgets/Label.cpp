#include "Label.h"
#include "CairoGUI.h"


void Label::paint()
{
	auto cairo = gui->cairo();
	cairo_save(cairo);

	cairo_select_font_face(cairo, (font ? font : gui->default_font()), CAIRO_FONT_SLANT_NORMAL, font_weight);
	cairo_set_font_size(cairo, rect.height);
	cairo_font_extents_t font_extents;
	cairo_font_extents(cairo, &font_extents);
	cairo_move_to(cairo, rect.x, rect.y + font_extents.ascent);
	cairo_set_source_rgb(cairo, color.red, color.green, color.blue);
	cairo_show_text(cairo, label.c_str());

	cairo_restore(cairo);
}


