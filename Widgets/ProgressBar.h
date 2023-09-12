#pragma once

#include "Widget.h"
#include "TimeSeconds.h"


class ProgressBar : public Widget {
	public:
		ProgressBar(CairoGUI* gui, Rect rect = {})
			: Widget(gui, rect) {}

		double current = 0.0, max = 0.0;
		Color border_color = { 0.0, 0.0, 0.0 };
		Color bar_color = { 0.0, 1.0, 0.0 };
		Color indefinite_color_1 = { 0.5, 1.0, 0.5 };
		Color indefinite_color_2 = { 0.75, 1.0, 0.75 };

		void paint();

	protected:
		TimeSeconds last_draw_time;
	};


