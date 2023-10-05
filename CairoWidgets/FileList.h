#pragma once

#include "Widget.h"
#include "TimeSeconds.h"
#include <cairo/cairo.h>
#include <vector>
#include <string>
#include <functional>

class CairoGUI;
class Scrollbar;


class FileList : public Widget {
	public:
		FileList(CairoGUI* gui, Rect rect);
		~FileList();

		std::function<bool(const char*)> file_filter;

		void set_dir(std::string path);
		void set_font(const char* font, double size, cairo_font_weight_t weight);

		void paint();
		void mouse_pressed(int x, int y);
		bool mouse_released(int x, int y);
		void mouse_moved(int x, int y);
		void scroll_down(int x, int y);
		void scroll_up(int x, int y) ;
		void key_pressed(std::string_view key);
		void special_key_pressed(std::string_view special_key);

		void resize_to(int width, int height);

		std::string selection_name();
		bool selection_is_dir();

		bool was_double_clicked = false;

	protected:
		struct Entry {
			std::string name;
			bool is_dir;
			};

		std::vector<Entry> entries;
		Scrollbar* scrollbar;
		bool needs_scrollbar_update = false;
		Widget* tracking_widget = nullptr;
		int selected_index = -1;
		TimeSeconds last_click_time;
		std::string search_string;
		const char* font = nullptr;
		double font_size = 16.0;
		cairo_font_weight_t font_weight = CAIRO_FONT_WEIGHT_NORMAL;

		Rect scrollbar_rect();
		void update_scrollbar();
		double get_line_height();
		int num_items_shown();
		void show_selected_item();
	};

