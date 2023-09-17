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
	file->read((char*) name.bytes, 2);

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
	: phdr_items(NULL), pbag_items(NULL), pmod_items(NULL), pgen_items(NULL),
		inst_items(NULL), ibag_items(NULL), imod_items(NULL), igen_items(NULL),
		shdr_items(NULL)
{
}


SF2::Hydra::~Hydra()
{
	delete phdr_items;
	delete pbag_items;
	delete pmod_items;
	delete pgen_items;
	delete inst_items;
	delete ibag_items;
	delete imod_items;
	delete igen_items;
	delete shdr_items;
}


void SF2::Hydra::read_from(std::istream* file, int64_t pdta_chunk_end)
{
	int i, num_items;

	#define handle_chunk(chunk_name) 	\
		if (fourcc_eq(chunk.id, #chunk_name)) { 	\
			num_items = chunk.size / SF2::chunk_name::size_in_file; 	\
			chunk_name##_num_items = num_items; 	\
			chunk_name##_items = new SF2::chunk_name[num_items]; 	\
			for (i = 0; i < num_items; ++i) 	\
				chunk_name##_items[i].read_from(file); 	\
			} 	\
		else

	while (file->tellg() < pdta_chunk_end) {
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


bool SF2::Hydra::is_complete()
{
	return
		phdr_items && pbag_items && pmod_items && pgen_items &&
		inst_items && ibag_items && imod_items && igen_items &&
		shdr_items;
}



