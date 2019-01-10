
#ifndef _INTERNALS_EXAMPLE_H
#define _INTERNALS_EXAMPLE_H

#include "InternalsPlugin.hpp"

#ifdef _AMD64_
#define PLUGIN_64BIT true
#else
#define PLUGIN_64BIT false
#endif

// Each component can be in [0:99] range.
#define PLUGIN_VERSION_MAJOR "1.0"
#define PLUGIN_VERSION_MINOR "1.0"
#define PLUGIN_NAME_AND_VERSION "rFactor 2 Dx 11 Screen Renderer" PLUGIN_VERSION_MAJOR
#define SHARED_MEMORY_VERSION PLUGIN_VERSION_MAJOR "." PLUGIN_VERSION_MINOR


class Dx11Plugin : public InternalsPluginV07  // REMINDER: exported function GetPluginVersion() should return 1 if you are deriving from this InternalsPluginV01, 2 for InternalsPluginV02, etc.
{
public:
	// Constructor/destructor
	Dx11Plugin() {}
	~Dx11Plugin() {}
	// These are the functions derived from base class InternalsPlugin
	// that can be implemented.
	void Startup(long version);  // game startup
	void UpdateTelemetry(const TelemInfoV01 &info);  // update plugin with telemetry info
	void InitializeHook();
private:
};
#endif // _INTERNALS_EXAMPLE_H