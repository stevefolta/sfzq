#include "SFZDebug.h"
#include <stdarg.h>

using namespace SFZero;

#ifdef JUCE_DEBUG

static LogFifo* fifo = NULL;


LogFifo::LogFifo()
	: fifo(capacity)
{
}


LogFifo::~LogFifo()
{
}


void LogFifo::log_message(const std::string& message)
{
	const char* p;

	// Stupid String class doesn't really let us get number of bytes.
	const char* bytes = message.getCharPointer();
	unsigned long msgSize = strlen(bytes);
	int totalSize = sizeof(unsigned long) + msgSize;
	int start1, size1, start2, size2;
	fifo.prepareToWrite(totalSize, start1, size1, start2, size2);
	int givenSize = size1 + size2;
	if (givenSize < totalSize)
		msgSize -= givenSize - totalSize;

	// Write the count.
	if (size1 >= sizeof(unsigned long)) {
		memcpy(&buffer[start1], &msgSize, sizeof(unsigned long));
		size1 -= sizeof(unsigned long);
		start1 += sizeof(unsigned long);
		}
	else {
		p = (const char*) &msgSize;
		memcpy(&buffer[start1], p, size1);
		p += size1;
		size1 = 0;
		int bytesLeft = sizeof(unsigned long) - size1;
		memcpy(&buffer[start2], p, bytesLeft);
		start2 += bytesLeft;
		size2 -= bytesLeft;
		}

	// Write the string.
	p = bytes;
	if (size1 > 0) {
		memcpy(&buffer[start1], p, size1);
		p += size1;
		}
	if (size2 > 0)
		memcpy(&buffer[start2], p, size2);

	fifo.finishedWrite(givenSize);
}


void LogFifo::relay_messages()
{
	while (has_message()) {
		std::string message = next_message();
		Logger::writeToLog(message);
		}
}


std::string LogFifo::next_message()
{
	// Read the count.
	unsigned long msgSize = 0;
	int start1, size1, start2, size2;
	fifo.prepareToRead(sizeof(unsigned long), start1, size1, start2, size2);
	char* p = (char*) &msgSize;
	if (size1 > 0) {
		memcpy(p, &buffer[start1], size1);
		p += size1;
		}
	if (size2 > 0)
		memcpy(p, &buffer[start2], size2);
	fifo.finishedRead(size1 + size2);

	// Read the string.
	std::string result;
	fifo.prepareToRead(msgSize, start1, size1, start2, size2);
	if (size1 > 0) {
		p = &buffer[start1];
		result = std::string(CharPointer_UTF8(p), CharPointer_UTF8(p + size1));
		}
	if (size2 > 0) {
		p = &buffer[start2];
		result += std::string(CharPointer_UTF8(p), CharPointer_UTF8(p + size2));
		}
	fifo.finishedRead(size1 + size2);

	return result;
}


bool LogFifo::has_message()
{
	return fifo.getNumReady() > 0;
}



void SFZero::setup_logging(Logger* logger)
{
	if (fifo == NULL)
		fifo = new LogFifo();
	Logger::setCurrentLogger(logger);
}


void SFZero::fifo_log_message(const std::string& message)
{
	if (fifo)
		fifo->log_message(message);
}


void SFZero::relay_fifo_log_messages()
{
	if (fifo)
		fifo->relay_messages();
}


void SFZero::dbgprintf(const char* msg, ...)
{
	va_list args;
	va_start(args, msg);

	char output[256];
	vsnprintf(output, 256, msg, args);
	fifo_log_message(output);

	va_end(args);
}


#endif 	// JUCE_DEBUG


