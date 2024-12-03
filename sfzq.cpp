#include "SFZQPlugin.h"
#include <vector>
#include <iostream>


static const std::vector<clap_plugin_descriptor_t> our_descriptors = {
	{
		.clap_version = CLAP_VERSION_INIT,
		.id = "net.stevefolta.sfzq",
		.name = "SFZQ",
		.vendor = "Steve Folta",
		.url = "https://github.com/stevefolta/sfzq",
		.manual_url = "",
		.support_url = "",
		.version = "1.0.0",
		.description = "SFZ/SF2 sample player",
		.features = (const char*[]) {
			CLAP_PLUGIN_FEATURE_INSTRUMENT,
			CLAP_PLUGIN_FEATURE_SAMPLER,
			CLAP_PLUGIN_FEATURE_STEREO,
			nullptr,
			},
		},
	};

class SFZQPluginFactory : public CLAPPluginFactory {
	public:
		const std::vector<clap_plugin_descriptor_t>& descriptors() {
			return our_descriptors;
			}

		CLAPPlugin* create_plugin(const clap_plugin_descriptor_t* descriptor, const clap_host_t* host) {
			return new SFZQPlugin(descriptor, host);
			}
	};
SFZQPluginFactory factory;

DECLARE_CLAP_PLUGIN_ENTRY(factory)


#ifdef DEBUG_UNLOADING
extern "C" __attribute__((destructor)) void at_unload()
{
	std::cerr << "- Unloading." << std::endl << std::endl;
}
#endif

