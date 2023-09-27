#pragma once

#include "Widget.h"
#include <cairo/cairo.h>
#include <string>


class Checkbox : public Widget {
	public:
		Checkbox(CairoGUI* gui_in, std::string text_in, Rect rect_in = {})
			: Widget(gui_in, rect_in), text(text_in) {}

		bool checked = false;
		bool enabled = true;
		std::string text;
		const char* font = nullptr;
		cairo_font_weight_t font_weight = CAIRO_FONT_WEIGHT_NORMAL;
		Color box_color = { 0.0, 0.0, 0.0 };
		Color text_color = { 0.0, 0.0, 0.0 };

		void paint();
		void mouse_pressed(int x, int y);
		bool mouse_released(int x, int y);
		void mouse_moved(int x, int y);

		double drawn_width();

	protected:
		bool pressed = false, hovering = false;
	};

