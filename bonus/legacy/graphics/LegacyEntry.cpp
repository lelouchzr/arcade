#include "LegacyGraphics.hpp"
#include <cstdlib>

extern "C" ARCADE_EXPORT const arcade::PluginDescriptor *arcade_plugin() {
    if (!std::getenv("ARCADE_LEGACY_GRAPHICS"))
        return nullptr;
    static const arcade::PluginDescriptor descriptor{
        arcade::AbiVersion,
        arcade::PluginKind::Graphics,
        "legacy_graphics",
        "HISTORICAL DISPLAY",
        []() -> arcade::IPlugin * { return new arcade::legacy::LegacyGraphics(); },
        [](arcade::IPlugin *plugin) noexcept { delete plugin; }};
    return &descriptor;
}
