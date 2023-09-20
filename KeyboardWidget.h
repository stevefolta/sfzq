#pragma once

#include "Widget.h"
#include <stdint.h>

class SFZSound;

class KeyboardWidget : public Widget {
	public:
		KeyboardWidget(CairoGUI* gui, Rect rect = {});

		void set_active_keys_0(uint64_t bitmap);
		void set_active_keys_1(uint64_t bitmap);
		void key_down(int key);
		void key_up(int key);

		void paint();

		Color white_key_color = { 1.0, 1.0, 1.0 };
		Color black_key_color = { 0.0, 0.0, 0.0 };
		Color inactive_key_color = { 0.75, 0.75, 0.75 };
		Color playing_key_color = { 0.5, 0.75, 1.0 };
		Color key_border_color = { 0.0, 0.0, 0.0 };

	protected:
		bool active_keys[128];
		bool playing_keys[128];
	};

