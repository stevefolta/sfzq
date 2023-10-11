#pragma once

#include "Widget.h"
#include <cairo/cairo.h>
#include <string>
#include <string_view>
#include <functional>

class FileList;
class Button;


class FileChooser : public Widget {
	public:
		FileChooser(CairoGUI* gui, Rect rect);
		~FileChooser();

		void set_path(std::string new_path);
		void go_to_cwd();
		void set_file_filter(std::function<bool(const char*)> new_file_filter);

		void set_ok_fn(std::function<void(std::string path)> fn) { ok_fn = fn; }
		void set_cancel_fn(std::function<void(void)> fn) { cancel_fn = fn; }
		void set_font(const char* font, double size, cairo_font_weight_t weight);
		void set_path_font(const char* font, double size, cairo_font_weight_t weight);
		void set_button_font(const char* font, cairo_font_weight_t weight);

		void paint();
		void mouse_pressed(int x, int y);
		bool mouse_released(int x, int y);
		void mouse_moved(int x, int y);
		void scroll_down(int x, int y);
		void scroll_up(int x, int y);
		void key_pressed(std::string_view key);
		void special_key_pressed(std::string_view special_key);

		void resize_to(int width, int height);

	protected:
		std::string path;
		FileList* file_list;
		Button* up_button;
		Button* ok_button;
		Button* cancel_button;
		Widget* tracking_widget = nullptr;
		std::function<void(std::string)> ok_fn;
		std::function<void(void)> cancel_fn;
		const char* path_font = nullptr;
		double path_font_size = 12.0;
		cairo_font_weight_t path_font_weight = CAIRO_FONT_WEIGHT_NORMAL;

		void layout();
		void draw_path();
		std::string cwd();
		void enter_selected_entry();
		void go_up();
	};

