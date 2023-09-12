#include "CLAPPosixFDExtension.h"
#include "CLAPPlugin.h"
#include "clap/clap.h"


CLAPPosixFDExtension::CLAPPosixFDExtension(CLAPPlugin* plugin_in)
	: plugin(plugin_in)
{
	host_posix_fd_support =
		(const clap_host_posix_fd_support_t*) plugin->host->get_extension(plugin->host, CLAP_EXT_POSIX_FD_SUPPORT);
}


const char* CLAPPosixFDExtension::clap_name()
{
	return CLAP_EXT_POSIX_FD_SUPPORT;
}


static const clap_plugin_posix_fd_support_t posix_fd_extension = {
	.on_fd = [](const clap_plugin_t* plugin, int fd, clap_posix_fd_flags_t flags) {
		CLAPPlugin::of(plugin)->on_fd(fd, flags);
		},
	};

const void* CLAPPosixFDExtension::clap_extension()
{
	return &posix_fd_extension;
}


void CLAPPosixFDExtension::register_fd(int fd, clap_posix_fd_flags_t flags)
{
	if (host_posix_fd_support && host_posix_fd_support->register_fd)
		host_posix_fd_support->register_fd(plugin->host, fd, flags);
}


void CLAPPosixFDExtension::unregister_fd(int fd)
{
	if (host_posix_fd_support && host_posix_fd_support->unregister_fd)
		host_posix_fd_support->unregister_fd(plugin->host, fd);
}




