#include "Sfml.hpp"
#include <SFML/Config.hpp>

namespace arcade {

std::vector<Input> Sfml::poll() {
    std::vector<Input> result;
#if SFML_VERSION_MAJOR >= 3
    while (const auto event = window_->pollEvent()) {
        if (const auto *key = event->getIf<sf::Event::KeyReleased>())
            if (key->code == sf::Keyboard::Key::Up || key->code == sf::Keyboard::Key::Down ||
                key->code == sf::Keyboard::Key::Left || key->code == sf::Keyboard::Key::Right)
                result.push_back({Action::Stop});
        if (event->is<sf::Event::Closed>())
            result.push_back({Action::Quit});
        if (const auto *value = event->getIf<sf::Event::TextEntered>())
            if (value->unicode > 32 && value->unicode < 127)
                result.push_back({Action::Text, static_cast<char>(value->unicode)});
        if (const auto *key = event->getIf<sf::Event::KeyPressed>())
            addKey(key->code, result);
    }
#else
    sf::Event event;
    while (window_->pollEvent(event)) {
        if (event.type == sf::Event::KeyReleased &&
            (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::Down ||
             event.key.code == sf::Keyboard::Left || event.key.code == sf::Keyboard::Right))
            result.push_back({Action::Stop});
        if (event.type == sf::Event::Closed)
            result.push_back({Action::Quit});
        if (event.type == sf::Event::TextEntered && event.text.unicode > 32 &&
            event.text.unicode < 127)
            result.push_back({Action::Text, static_cast<char>(event.text.unicode)});
        if (event.type == sf::Event::KeyPressed)
            addKey(event.key.code, result);
    }
#endif
    return result;
}

void Sfml::addKey(sf::Keyboard::Key key, std::vector<Input> &result) {
#if SFML_VERSION_MAJOR >= 3
    using K = sf::Keyboard::Key;
#else
    using K = sf::Keyboard;
#endif
    Action action = Action::None;
    switch (key) {
    case K::Up:
        action = Action::Up;
        break;
    case K::Down:
        action = Action::Down;
        break;
    case K::Left:
        action = Action::Left;
        break;
    case K::Right:
        action = Action::Right;
        break;
    case K::Enter:
        action = Action::Confirm;
        break;
    case K::Space:
        action = Action::Primary;
        break;
    case K::Backspace:
        action = Action::Backspace;
        break;
    case K::Escape:
        action = Action::Quit;
        break;
    case K::F2:
    case K::Tab:
        action = Action::NextGraphics;
        break;
    case K::F3:
        action = Action::NextGame;
        break;
    case K::F4:
        action = Action::Restart;
        break;
    case K::F5:
        action = Action::Menu;
        break;
    case K::F6:
        action = Action::Pause;
        break;
    case K::F7:
        action = Action::Difficulty;
        break;
    default:
        break;
    }
    if (action != Action::None)
        result.push_back({action});
}

} // namespace arcade
