#pragma once

#include "Widget.h"
#include <functional>

class Label;
class Button;
class SFZSound;


class SubsoundWidget : public Widget {
	public:
		SubsoundWidget(CairoGUI* gui_in, SFZSound* sound_in, Rect rect_in = {});
		~SubsoundWidget();

		std::function<void(int which_subsound)> select_subsound_fn;

		void paint();
		void mouse_pressed(int x, int y);
		bool mouse_released(int x, int y);
		void mouse_moved(int x, int y);

		void layout();
		void update();

	protected:
		SFZSound* sound;
		Label* name_label;
		Button* minus_button;
		Button* plus_button;
		Button* minus_more_button = nullptr;
		Button* plus_more_button = nullptr;
		Widget* tracking_widget = nullptr;
	};

