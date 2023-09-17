#pragma once

#include "WinTypes.h"
#include <istream>
#include <stdint.h>


#define SF2Field(type, name) 	\
	type name;

namespace SF2 {

	struct rangesType {
		byte lo, hi;
		};

	union genAmountType {
		rangesType range;
		short shortAmount;
		word wordAmount;
		};

	struct iver {
		#include "sf2-chunks/iver.h"
		void read_from(std::istream* file);
		};

	struct phdr {
		#include "sf2-chunks/phdr.h"
		void read_from(std::istream* file);
		static const int size_in_file = 38;
		};

	struct pbag {
		#include "sf2-chunks/pbag.h"
		void read_from(std::istream* file);
		static const int size_in_file = 4;
		};

	struct pmod {
		#include "sf2-chunks/pmod.h"
		void read_from(std::istream* file);
		static const int size_in_file = 10;
		};

	struct pgen {
		#include "sf2-chunks/pgen.h"
		void read_from(std::istream* file);
		static const int size_in_file = 4;
		};

	struct inst {
		#include "sf2-chunks/inst.h"
		void read_from(std::istream* file);
		static const int size_in_file = 22;
		};

	struct ibag {
		#include "sf2-chunks/ibag.h"
		void read_from(std::istream* file);
		static const int size_in_file = 4;
		};

	struct imod {
		#include "sf2-chunks/imod.h"
		void read_from(std::istream* file);
		static const int size_in_file = 10;
		};

	struct igen {
		#include "sf2-chunks/igen.h"
		void read_from(std::istream* file);
		static const int size_in_file = 4;
		};

	struct shdr {
		#include "sf2-chunks/shdr.h"
		void read_from(std::istream* file);
		static const int size_in_file = 46;
		};


	struct Hydra {
		phdr* phdr_items;
		pbag* pbag_items;
		pmod* pmod_items;
		pgen* pgen_items;
		inst* inst_items;
		ibag* ibag_items;
		imod* imod_items;
		igen* igen_items;
		shdr* shdr_items;

		int phdr_num_items, pbag_num_items, pmod_num_items, pgen_num_items;
		int inst_num_items, ibag_num_items, imod_num_items, igen_num_items;
		int shdr_num_items;

		Hydra();
		~Hydra();

		void read_from(std::istream* file, int64_t pdta_chunk_end);
		bool is_complete();
		};

	};

#undef SF2Field

