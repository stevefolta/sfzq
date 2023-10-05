#pragma once

#include <cairo/cairo.h>


class CairoGUI {
	public:
		virtual cairo_t* cairo() = 0;
		virtual void refresh() = 0;
		virtual const char* default_font() { return "Deja Vu Sans"; }
	};

