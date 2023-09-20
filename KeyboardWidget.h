#pragma once

#include "Widget.h"


class KeyboardWidget : public Widget {
	public:
		KeyboardWidget(CairoGUI* gui, Rect rect = {})
			: Widget(gui, rect) {}

		void paint();

		Color white_key_color = { 1.0, 1.0, 1.0 };
		Color black_key_color = { 0.0, 0.0, 0.0 };
		Color key_border_color = { 0.0, 0.0, 0.0 };

	protected:
	};

