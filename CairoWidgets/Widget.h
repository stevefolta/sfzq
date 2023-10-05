#pragma once

class CairoGUI;


struct Rect {
	double x, y;
	double width, height;
	};

struct Color {
	double red, green, blue, alpha = 1.0;
	};


class Widget {
	public:
		Widget(CairoGUI* gui_in, Rect rect_in)
			: rect(rect_in), gui(gui_in) {}
		virtual ~Widget() {}

		Rect rect;
		bool contains(double x, double y);

		virtual void paint() {}
		virtual void mouse_pressed(int x, int y) {}
		virtual bool mouse_released(int x, int y) { return false; }
		virtual void mouse_moved(int x, int y) {}
		virtual void scroll_down(int x, int y) {}
		virtual void scroll_up(int x, int y) {};

		void move_right_to(double right) {
			rect.x = right - rect.width;
			}
		void move_bottom_to(double bottom) {
			rect.y = bottom - rect.height;
			}

	protected:
		CairoGUI* gui;

		// Utilities.
		void rounded_rect(Rect rect, double corner_size);
		void rounded_rect(Rect rect, double corner_width, double corner_height);
		void use_color(const Color& color);
	};

