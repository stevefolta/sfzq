#include "RIFF.h"


void RIFFChunk::read_from(std::istream* file)
{
	file->read(&id[0], sizeof(fourcc));
	*file >> size;
	start = file->tellg();

	if (fourcc_eq(id, "RIFF")) {
		type = RIFF;
		file->read(&id[0], sizeof(fourcc));
		start += sizeof(fourcc);
		size -= sizeof(fourcc);
		}
	else if (fourcc_eq(id, "LIST")) {
		type = LIST;
		file->read(&id[0], sizeof(fourcc));
		start += sizeof(fourcc);
		size -= sizeof(fourcc);
		}
	else
		type = Custom;
}


void RIFFChunk::seek(std::istream* file)
{
	file->seekg(start);
}


void RIFFChunk::seek_after(std::istream* file)
{
	int64_t next = start + size;
	if (next % 2 != 0)
		next += 1;
	file->seekg(next);
}


std::string RIFFChunk::read_string(std::istream* file)
{
	char str[size];
	file->read(str, size);
	return std::string(str);
}




