#include "LegacyGraphics.hpp"
#include <cstdlib>
#include <stdexcept>
#include <unistd.h>

namespace arcade::legacy {

std::filesystem::path LegacyGraphics::configuredPath() {
    const char *path = std::getenv("ARCADE_LEGACY_GRAPHICS");
    if (!path || !*path)
        throw std::runtime_error("set ARCADE_LEGACY_GRAPHICS to a historical graphics module");
    return path;
}

LegacyProfile LegacyGraphics::configuredProfile() {
    const char *value = std::getenv("ARCADE_LEGACY_PROFILE");
    if (!value || std::string(value) == "terminal")
        return LegacyProfile::Terminal;
    if (std::string(value) == "raw")
        return LegacyProfile::RawText;
    if (std::string(value) == "sdl")
        return LegacyProfile::Sdl;
    if (std::string(value) == "sfml")
        return LegacyProfile::Sfml;
    throw std::invalid_argument("ARCADE_LEGACY_PROFILE must be terminal, raw, sdl or sfml");
}

LegacyGraphics::LegacyGraphics() : LegacyGraphics(configuredPath()) {}

LegacyGraphics::LegacyGraphics(std::filesystem::path path)
    : path_(std::move(path)), profile_(configuredProfile()) {}

LegacyGraphics::~LegacyGraphics() {
    close();
}

void LegacyGraphics::open() {
    if (display_)
        return;
    if (profile_ == LegacyProfile::Terminal) {
        const char *term = std::getenv("TERM");
        if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO) || !term || !*term ||
            std::string(term) == "dumb")
            throw std::runtime_error(
                "historical terminal graphics needs a real tty and valid TERM");
    }
    auto library = std::make_unique<SharedLibrary>(path_);
    (void)library->symbol<void (*)()>("createGraphic");
    const auto factory = library->symbol<Arcade::SharedInterfaces::IGraphic *(*)()>("entryPoint");
    auto display = std::unique_ptr<Arcade::SharedInterfaces::IGraphic>(factory());
    if (!display)
        throw std::runtime_error("historical graphics factory returned null");
    library_ = std::move(library);
    display_ = std::move(display);
}

void LegacyGraphics::close() noexcept {
    display_.reset();
    library_.reset();
}

} // namespace arcade::legacy
