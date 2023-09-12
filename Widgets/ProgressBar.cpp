#include "ProgressBar.h"
#include "CairoGUI.h"
#include <math.h>


void ProgressBar::paint()
{
	auto cairo = gui->cairo();
	cairo_save(cairo);

	// Bar.
	if (max > 0.0) {
		cairo_rectangle(cairo, rect.x, rect.y, rect.width * current / max, rect.height);
		use_color(bar_color);
		cairo_fill(cairo);
		}
	else {
		cairo_rectangle(cairo, rect.x, rect.y, rect.width, rect.height);
		double start_x = rect.x;
		double now = TimeSeconds::now().as_double();
		start_x -= (1.0 / 2 + 2 * (now - trunc(now))) * rect.width;
		cairo_pattern_t* pattern = cairo_pattern_create_linear(start_x, rect.y, start_x + 4 * rect.width, rect.y);
		cairo_pattern_add_color_stop_rgb(pattern, 0.0, indefinite_color_1.red, indefinite_color_1.green, indefinite_color_1.blue);
		cairo_pattern_add_color_stop_rgb(pattern, 1 / 4.0, indefinite_color_2.red, indefinite_color_2.green, indefinite_color_2.blue);
		cairo_pattern_add_color_stop_rgb(pattern, 2 / 4.0, indefinite_color_1.red, indefinite_color_1.green, indefinite_color_1.blue);
		cairo_pattern_add_color_stop_rgb(pattern, 3 / 4.0, indefinite_color_2.red, indefinite_color_2.green, indefinite_color_2.blue);
		cairo_pattern_add_color_stop_rgb(pattern, 4 / 4.0, indefinite_color_1.red, indefinite_color_1.green, indefinite_color_1.blue);
		cairo_set_source(cairo, pattern);
		cairo_fill(cairo);
		cairo_pattern_destroy(pattern);
		}

	// Border.
	cairo_rectangle(cairo, rect.x, rect.y, rect.width, rect.height);
	cairo_set_line_width(cairo, 1.0);
	use_color(border_color);
	cairo_stroke(cairo);

	cairo_restore(cairo);
}



