#pragma once

#include "CLAPExtension.h"
#include "clap/clap.h"

class CLAPPlugin;


class CLAPPosixFDExtension : public CLAPExtension {
	public:
		CLAPPosixFDExtension(CLAPPlugin* plugin_in);

		const char* clap_name();
		const void* clap_extension();

		void register_fd(int fd, clap_posix_fd_flags_t flags);
		void unregister_fd(int fd);

	protected:
		CLAPPlugin* plugin;
		const clap_host_posix_fd_support_t* host_posix_fd_support = nullptr;
	};

