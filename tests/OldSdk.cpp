#include "arcade/Api.hpp"

extern "C" ARCADE_EXPORT const arcade::PluginDescriptor *arcade_plugin() {
    static const arcade::PluginDescriptor descriptor{2,
                                                     arcade::PluginKind::Graphics,
                                                     "old_sdk",
                                                     "OLD SDK",
                                                     []() -> arcade::IPlugin * { return nullptr; },
                                                     [](arcade::IPlugin *) noexcept {}};
    return &descriptor;
}
