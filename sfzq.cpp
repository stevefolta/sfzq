#include "SFZQPlugin.h"
#include <vector>


static const std::vector<clap_plugin_descriptor_t> our_descriptors = {
	{
		.clap_version = CLAP_VERSION_INIT,
		.id = "net.stevefolta.sfzq",
		.name = "SFZQ",
		.vendor = "Steve Folta",
		.features = (const char*[]) {
			CLAP_PLUGIN_FEATURE_INSTRUMENT,
			CLAP_PLUGIN_FEATURE_SYNTHESIZER,
			CLAP_PLUGIN_FEATURE_STEREO,
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

