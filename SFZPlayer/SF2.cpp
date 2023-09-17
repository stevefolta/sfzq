#include "SF2.h"
#include "RIFF.h"


static char read_char(std::istream* file)
{
	char c;
	file->read(&c, 1);
	return c;
}

static dword read_dword(std::istream* file)
{
	uint8_t bytes[4];
	file->read((char*) bytes, sizeof(bytes));
	return bytes[0] | (dword) bytes[1] << 8 | (dword) bytes[2] << 16 | (dword) bytes[3] << 24;
}

static word read_word(std::istream* file)
{
	uint8_t bytes[2];
	file->read((char*) bytes, sizeof(bytes));
	return bytes[0] | (word) bytes[1] << 8;
}

static int16_t read_short(std::istream* file)
{
	uint8_t bytes[2];
	file->read((char*) bytes, sizeof(bytes));
	return (int16_t) (bytes[0] | (uint16_t) bytes[1]) << 8;
}

#define read_a_byte(name, file) 	\
	name = (byte) read_char(file);
#define read_a_char(name, file) 	\
	name = read_char(file);
#define read_a_dword(name, file) 	\
	name = read_dword(file);
#define read_a_word(name, file) 	\
	name = read_word(file);
#define read_a_short(name, file) 	\
	name = read_short(file);
#define read_a_char20(name, file) 	\
	file->read(name, 20);
#define read_a_genAmountType(name, file) 	\
	name.shortAmount = read_short(file);

#define SF2Field(type, name) 	\
	read_a_##type(name, file)


void SF2::iver::read_from(std::istream* file)
{
	#include "sf2-chunks/iver.h"
}


void SF2::phdr::read_from(std::istream* file)
{
	#include "sf2-chunks/phdr.h"
}


void SF2::pbag::read_from(std::istream* file)
{
	#include "sf2-chunks/pbag.h"
}


void SF2::pmod::read_from(std::istream* file)
{
	#include "sf2-chunks/pmod.h"
}


void SF2::pgen::read_from(std::istream* file)
{
	#include "sf2-chunks/pgen.h"
}


void SF2::inst::read_from(std::istream* file)
{
	#include "sf2-chunks/inst.h"
}


void SF2::ibag::read_from(std::istream* file)
{
	#include "sf2-chunks/ibag.h"
}


void SF2::imod::read_from(std::istream* file)
{
	#include "sf2-chunks/imod.h"
}


void SF2::igen::read_from(std::istream* file)
{
	#include "sf2-chunks/igen.h"
}


void SF2::shdr::read_from(std::istream* file)
{
	#include "sf2-chunks/shdr.h"
}


SF2::Hydra::Hydra()
	: phdrItems(NULL), pbagItems(NULL), pmodItems(NULL), pgenItems(NULL),
		instItems(NULL), ibagItems(NULL), imodItems(NULL), igenItems(NULL),
		shdrItems(NULL)
{
}


SF2::Hydra::~Hydra()
{
	delete phdrItems;
	delete pbagItems;
	delete pmodItems;
	delete pgenItems;
	delete instItems;
	delete ibagItems;
	delete imodItems;
	delete igenItems;
	delete shdrItems;
}


void SF2::Hydra::read_from(std::istream* file, int64_t pdtaChunkEnd)
{
	int i, num_items;

	#define handle_chunk(chunkName) 	\
		if (fourcc_eq(chunk.id, #chunkName)) { 	\
			num_items = chunk.size / SF2::chunkName::sizeInFile; 	\
			chunkName##NumItems = num_items; 	\
			chunkName##Items = new SF2::chunkName[num_items]; 	\
			for (i = 0; i < num_items; ++i) 	\
				chunkName##Items[i].read_from(file); 	\
			} 	\
		else

	while (file->tellg() < pdtaChunkEnd) {
		RIFFChunk chunk;
		chunk.read_from(file);

		handle_chunk(phdr)
		handle_chunk(pbag)
		handle_chunk(pmod)
		handle_chunk(pgen)
		handle_chunk(inst)
		handle_chunk(ibag)
		handle_chunk(imod)
		handle_chunk(igen)
		handle_chunk(shdr)
		{}

		chunk.seek_after(file);
		}
}


bool SF2::Hydra::IsComplete()
{
	return
		phdrItems && pbagItems && pmodItems && pgenItems &&
		instItems && ibagItems && imodItems && igenItems &&
		shdrItems;
}



