#include "TextBox.h"
#include "CairoGUI.h"
#include <string_view>


void TextBox::paint()
{
	auto cairo = gui->cairo();
	cairo_save(cairo);
	cairo_rectangle(cairo, rect.x, rect.y, rect.width, rect.height);
	cairo_clip(cairo);

	cairo_select_font_face(cairo, (font ? font : gui->default_font()), CAIRO_FONT_SLANT_NORMAL, font_weight);
	cairo_set_font_size(cairo, font_size);
	cairo_set_source_rgb(cairo, color.red, color.green, color.blue);
	cairo_font_extents_t font_extents;
	cairo_font_extents(cairo, &font_extents);

	// Draw line-by-line.
	std::string_view remainder = rtrim(text);
	double baseline = rect.y + font_extents.ascent;
	while (!remainder.empty()) {
		if (baseline - font_extents.ascent > rect.y + rect.height)
			break;

		// Get the next line.
		std::string_view line;
		auto pos = remainder.find('\n');
		if (pos == std::string_view::npos) {
			line = remainder;
			remainder = "";
			}
		else {
			line = remainder.substr(0, pos);
			remainder = remainder.substr(pos + 1);
			}
		line = rtrim(line);

		// Does it fit?
		cairo_text_extents_t text_extents;
		std::string line_str(line);
		cairo_text_extents(cairo, line_str.c_str(), &text_extents);
		if (text_extents.width <= rect.width) {
			cairo_move_to(cairo, rect.x, baseline);
			cairo_show_text(cairo, line_str.c_str());
			baseline += font_extents.height;
			}
		else {
			// No, wrap the text.
			auto line_remainder = line;
			while (!line_remainder.empty()) {
				// Binary search for the maximum that will fit.
				int max_fit = 0;
				int left = 0;
				int right = line.size() - 1;
				while (left <= right) {
					int middle = (left + right) / 2;
					auto adjusted_middle = line_remainder.find_first_of(" \t", middle);
					if (adjusted_middle == std::string_view::npos)
						adjusted_middle = line_remainder.size();
					cairo_text_extents(
						cairo,
						std::string(line_remainder.substr(0, adjusted_middle)).c_str(),
						&text_extents);
					if (text_extents.width <= rect.width) {
						max_fit = adjusted_middle;
						left = adjusted_middle + 1;
						if (left >= (int) line_remainder.size())
							break;
						}
					else
						right = middle - 1;
					}

				if (max_fit == 0) {
					// Just draw the whole remainder.
					max_fit = line_remainder.size();
					}

				// Draw that.
				cairo_move_to(cairo, rect.x, baseline);
				cairo_show_text(cairo, std::string(line_remainder.substr(0, max_fit)).c_str());
				baseline += font_extents.height;

				auto next_word_start = line_remainder.find_first_not_of(" \t", max_fit);
				if (next_word_start == std::string_view::npos)
					break;
				line_remainder = line_remainder.substr(next_word_start);
				}
			}
		}

	// Were there more lines than fit?
	if (baseline - font_extents.height + font_extents.descent > rect.y + rect.height) {
		// Fade it out.
		double fade_top = rect.y + rect.height - font_extents.height;
		cairo_rectangle(cairo, rect.x, fade_top, rect.width, font_extents.height);
		cairo_pattern_t* pattern = cairo_pattern_create_linear(rect.x, fade_top, rect.x, rect.y + rect.height);
		cairo_pattern_add_color_stop_rgba(pattern, 0.0, 1.0, 1.0, 1.0, 0.0);
		cairo_pattern_add_color_stop_rgba(pattern, 1.0, 1.0, 1.0, 1.0, 1.0);
		cairo_set_source(cairo, pattern);
		cairo_fill(cairo);
		cairo_pattern_destroy(pattern);
		}

	cairo_restore(cairo);
}


std::string_view TextBox::rtrim(const std::string_view& str)
{
	auto last_pos = str.find_last_not_of(" \t\r\n");
	if (last_pos == std::string_view::npos)
		return "";
	return str.substr(0, last_pos + 1);
}



