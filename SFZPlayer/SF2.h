#pragma once

#include "WinTypes.h"
#include <istream>
#include <stdint.h>


#define SF2Field(type, name) 	\
	type	name;

namespace SF2 {

	struct rangesType {
		byte	lo, hi;
		};

	union genAmountType {
		rangesType	range;
		short     	shortAmount;
		word      	wordAmount;
		};

	struct iver {
		#include "sf2-chunks/iver.h"
		void	read_from(std::istream* file);
		};

	struct phdr {
		#include "sf2-chunks/phdr.h"
		void	read_from(std::istream* file);
		static const int sizeInFile = 38;
		};

	struct pbag {
		#include "sf2-chunks/pbag.h"
		void	read_from(std::istream* file);
		static const int sizeInFile = 4;
		};

	struct pmod {
		#include "sf2-chunks/pmod.h"
		void	read_from(std::istream* file);
		static const int	sizeInFile = 10;
		};

	struct pgen {
		#include "sf2-chunks/pgen.h"
		void	read_from(std::istream* file);
		static const int	sizeInFile = 4;
		};

	struct inst {
		#include "sf2-chunks/inst.h"
		void	read_from(std::istream* file);
		static const int	sizeInFile = 22;
		};

	struct ibag {
		#include "sf2-chunks/ibag.h"
		void	read_from(std::istream* file);
		static const int	sizeInFile = 4;
		};

	struct imod {
		#include "sf2-chunks/imod.h"
		void	read_from(std::istream* file);
		static const int	sizeInFile = 10;
		};

	struct igen {
		#include "sf2-chunks/igen.h"
		void	read_from(std::istream* file);
		static const int	sizeInFile = 4;
		};

	struct shdr {
		#include "sf2-chunks/shdr.h"
		void	read_from(std::istream* file);
		static const int	sizeInFile = 46;
		};


	struct Hydra {
		phdr*	phdrItems;
		pbag*	pbagItems;
		pmod*	pmodItems;
		pgen*	pgenItems;
		inst*	instItems;
		ibag*	ibagItems;
		imod*	imodItems;
		igen*	igenItems;
		shdr*	shdrItems;

		int	phdrNumItems, pbagNumItems, pmodNumItems, pgenNumItems;
		int	instNumItems, ibagNumItems, imodNumItems, igenNumItems;
		int	shdrNumItems;

		Hydra();
		~Hydra();

		void	read_from(std::istream* file, int64_t pdtaChunkEnd);
		bool	IsComplete();
		};

	};

#undef SF2Field

