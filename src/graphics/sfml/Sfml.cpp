#include "Sfml.hpp"
#include <SFML/Config.hpp>
#include <stdexcept>

namespace arcade {

Sfml::~Sfml() {
    close();
}

void Sfml::open() {
    if (window_)
        return;
#if SFML_VERSION_MAJOR >= 3
    window_ = std::make_unique<sf::RenderWindow>(sf::VideoMode({Frame::Width, Frame::Height}),
                                                 "ARCADE / 400 - SFML");
#else
    window_ = std::make_unique<sf::RenderWindow>(sf::VideoMode(Frame::Width, Frame::Height),
                                                 "ARCADE / 400 - SFML");
#endif
    if (!window_->isOpen()) {
        window_.reset();
        throw std::runtime_error("SFML cannot open a window");
    }
    window_->setKeyRepeatEnabled(true);
}

void Sfml::close() noexcept {
    if (window_) {
        window_->close();
        window_.reset();
    }
}

} // namespace arcade
ARCADE_PLUGIN(arcade::Sfml, arcade::PluginKind::Graphics, "sfml", "SFML / VECTOR ENGINE")
