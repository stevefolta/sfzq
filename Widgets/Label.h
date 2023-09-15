#pragma once

#include "Widget.h"
#include <cairo/cairo.h>
#include <string>


class Label : public Widget {
	public:
		Label(CairoGUI* gui, std::string label_in, Rect rect = {})
			: Widget(gui, rect), label(label_in) {}

		std::string label;
		Color color = { 0.0, 0.0, 0.0 };
		const char* font = nullptr;
		cairo_font_weight_t font_weight = CAIRO_FONT_WEIGHT_BOLD;

		virtual void paint();
	};

