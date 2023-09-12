#pragma once

#include <string>

class Logger; 	//*** TODO


#ifdef DEBUG
	#define SFZDBG(msg)	fifo_log_message(msg)
	#define SHOW(item)	SFZDBG( #item " = " + std::string(item) )
#else
	#define	SFZDBG(msg)
	#define	SHOW(msg)
#endif


class LogFifo {
	public:
		LogFifo();
		~LogFifo();

		void log_message(const std::string& message);
		void relay_messages();
		std::string next_message();
		bool has_message();

	protected:
		enum {
			capacity = 512 * 1024,
			};
		//***TODO AbstractFifo	fifo;
		char	buffer[capacity];
	};

extern void setup_logging(Logger* logger);
extern void fifo_log_message(const std::string& message);
extern void relay_fifo_log_messages();

extern void dbgprintf(const char* msg, ...);


