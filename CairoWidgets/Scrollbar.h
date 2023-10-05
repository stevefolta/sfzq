#pragma once

#include "Widget.h"
#include <functional>


class Scrollbar : public Widget {
	public:
		Scrollbar(CairoGUI* gui, Rect rect)
			: Widget(gui, rect) {}

		double max = 0.0, value = 0.0;
		double percentage = 0.0; 	// A misnomer; this is in the range 0..1.

		void paint();
		void mouse_pressed(int x, int y);
		bool mouse_released(int x, int y);
		void mouse_moved(int x, int y);

		void set_value_changed(std::function<void(double)> f) { value_changed = f; }

	protected:
		std::function<void(double)> value_changed;
		bool scrolling = false;
		double start_value;
		int start_y;
	};

