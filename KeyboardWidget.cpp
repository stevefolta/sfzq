#include "KeyboardWidget.h"
#include "CairoGUI.h"
#include "SFZSound.h"
#include <string.h>
#include <iostream>

static const double black_key_height_factor = 0.666;
static const double black_key_width_factor = 0.5;
static const double key_stroke_width = 0.5;


KeyboardWidget::KeyboardWidget(CairoGUI* gui, Rect rect)
	: Widget(gui, rect)
{
	memset(active_keys, 0, sizeof(active_keys));
}


void KeyboardWidget::use_sound(SFZSound* sound)
{
	memset(active_keys, 0, sizeof(active_keys));
	if (sound == nullptr)
		return;

	for (int key = 0; key < 128; ++key) {
		if (sound->get_region_for(key, 64, 0.5))
			active_keys[key] = true;
		}
}


void KeyboardWidget::paint()
{
	auto cairo = gui->cairo();
	cairo_save(cairo);

	// Draw white keys.
	double key_width = rect.width / 52;
	Rect key_rect = { rect.x, rect.y, key_width, rect.height };
	for (int key = 21; key <= 108; ++key) {
		// Is it a white key?
		int octave_note = key % 12;
		if (octave_note == 1 || octave_note == 3 || octave_note == 6 || octave_note == 8 || octave_note == 10)
			continue;
		// Draw.
		cairo_rectangle(cairo, key_rect.x, key_rect.y, key_rect.width, key_rect.height);
		if (active_keys[key])
			cairo_set_source_rgba(cairo, white_key_color.red, white_key_color.green, white_key_color.blue, white_key_color.alpha);
		else
			cairo_set_source_rgba(cairo, inactive_key_color.red, inactive_key_color.green, inactive_key_color.blue, inactive_key_color.alpha);
		cairo_fill_preserve(cairo);
		cairo_set_source_rgba(cairo, key_border_color.red, key_border_color.green, key_border_color.blue, key_border_color.alpha);
		cairo_set_line_width(cairo, key_stroke_width);
		cairo_stroke(cairo);
		// Next key.
		key_rect.x += key_width;
		}

	// Draw black keys.
	double black_key_width = key_width * black_key_width_factor;
	key_rect = { rect.x + key_width - (key_width - black_key_width) / 2, rect.y, black_key_width, rect.height * black_key_height_factor };
	for (int key = 21; key <= 108; ++key) {
		// Is it a black key?
		int octave_note = key % 12;
		if (octave_note != 1 && octave_note != 3 && octave_note != 6 && octave_note != 8 && octave_note != 10)
			continue;
		// Draw.
		cairo_rectangle(cairo, key_rect.x, key_rect.y, key_rect.width, key_rect.height);
		if (active_keys[key])
			cairo_set_source_rgba(cairo, black_key_color.red, black_key_color.green, black_key_color.blue, black_key_color.alpha);
		else
			cairo_set_source_rgba(cairo, inactive_key_color.red, inactive_key_color.green, inactive_key_color.blue, inactive_key_color.alpha);
		cairo_fill_preserve(cairo);
		cairo_set_source_rgba(cairo, key_border_color.red, key_border_color.green, key_border_color.blue, key_border_color.alpha);
		cairo_set_line_width(cairo, key_stroke_width);
		cairo_stroke(cairo);
		// Next key.
		if (octave_note == 3 || octave_note == 10)
			key_rect.x += key_width * 2;
		else
			key_rect.x += key_width;
		}

	/***/

	cairo_restore(cairo);
}



