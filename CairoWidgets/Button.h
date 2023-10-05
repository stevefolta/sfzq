#pragma once

#include "Widget.h"
#include <cairo/cairo.h>


class Button : public Widget {
	public:
		Button(
			CairoGUI* gui,
			Rect rect,
			const char* label_in)
			: Widget(gui, rect), label(label_in)
		{}
		Button(CairoGUI* gui, const char* label_in, Rect rect = {})
			: Widget(gui, rect), label(label_in) {}

		void set_font(const char* font_in, cairo_font_weight_t weight_in) {
			font = font_in;
			font_weight = weight_in;
			}
		bool enabled = true;

		void paint();
		void mouse_pressed(int x, int y);
		bool mouse_released(int x, int y);
		void mouse_moved(int x, int y);

	protected:
		const char* label;
		bool pressed = false, is_mouse_over = false;
		const char* font = nullptr;
		cairo_font_weight_t font_weight = CAIRO_FONT_WEIGHT_BOLD;
	};

