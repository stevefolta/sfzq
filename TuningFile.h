#pragma once

#include "Widget.h"
#include <string>
#include <vector>

class SFZQPlugin;
class Checkbox;
class Label;


class TuningFile : public Widget {
	public:
		TuningFile()
			: Widget(nullptr, {}) {}
		void init(CairoGUI* gui, SFZQPlugin* plugin, std::string initial_label, std::string main_label);
		~TuningFile();

		void paint();
		void mouse_pressed(int x, int y);
		bool mouse_released(int x, int y);
		void mouse_moved(int x, int y);

		void layout();

		void enable();
		void update();
		void set_path(std::string new_path);

		bool enabled = false;
		std::string path;
		std::vector<std::string> extensions;

	protected:
		SFZQPlugin* plugin;
		std::string main_label;
		Checkbox* checkbox = nullptr;
		Label* label = nullptr;
		Widget* tracking_widget = nullptr;
	};

