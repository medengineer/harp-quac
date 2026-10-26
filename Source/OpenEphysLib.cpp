#include <PluginInfo.h>

#include "HarpQuacNode.h"

#include <string>

#ifdef WIN32
#include <Windows.h>
#define EXPORT __declspec (dllexport)
#else
#define EXPORT __attribute__ ((visibility ("default")))
#endif

using namespace Plugin;

#define NUM_PLUGINS 1

extern "C" EXPORT void getLibInfo (Plugin::LibraryInfo* info)
{
    info->apiVersion = PLUGIN_API_VER;
    info->name = "Harp Quac";
    info->libVersion = "0.1.0";
    info->numPlugins = NUM_PLUGINS;
}

extern "C" EXPORT int getPluginInfo (int index, Plugin::PluginInfo* info)
{
    switch (index)
    {
        case 0:
            info->type = Plugin::Type::PROCESSOR;
            info->processor.name = "Harp Quac";
            info->processor.type = Processor::Type::SINK;
            info->processor.creator = &(Plugin::createProcessor<HarpQuacNode>);
            break;
        default:
            return -1;
    }
    return 0;
}

#ifdef WIN32
BOOL WINAPI DllMain (IN HINSTANCE, IN DWORD, IN LPVOID)
{
    return TRUE;
}
#endif
