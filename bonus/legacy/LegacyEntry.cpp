#include "LegacyGame.hpp"
#include <cstdlib>

extern "C" ARCADE_EXPORT const arcade::PluginDescriptor *arcade_plugin() {
    if (!std::getenv("ARCADE_LEGACY_GAME"))
        return nullptr;
    static const arcade::PluginDescriptor descriptor{
        arcade::AbiVersion,
        arcade::PluginKind::Game,
        "legacy",
        "HISTORICAL GAME",
        []() -> arcade::IPlugin * { return new arcade::legacy::LegacyGame(); },
        [](arcade::IPlugin *plugin) noexcept { delete plugin; }};
    return &descriptor;
}
