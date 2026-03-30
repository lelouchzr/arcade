#include "Sdl2.hpp"

namespace arcade {

std::vector<Input> Sdl2::poll() {
    std::vector<Input> result;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT)
            result.push_back({Action::Quit});
        if (event.type == SDL_TEXTINPUT)
            for (char value : std::string(event.text.text))
                if (value > 32 && value < 127)
                    result.push_back({Action::Text, value});
        if (event.type == SDL_KEYUP &&
            (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_DOWN ||
             event.key.keysym.sym == SDLK_LEFT || event.key.keysym.sym == SDLK_RIGHT))
            result.push_back({Action::Stop});
        if (event.type == SDL_KEYDOWN) {
            Action action = Action::None;
            switch (event.key.keysym.sym) {
            case SDLK_UP:
                action = Action::Up;
                break;
            case SDLK_DOWN:
                action = Action::Down;
                break;
            case SDLK_LEFT:
                action = Action::Left;
                break;
            case SDLK_RIGHT:
                action = Action::Right;
                break;
            case SDLK_RETURN:
                action = Action::Confirm;
                break;
            case SDLK_SPACE:
                action = Action::Primary;
                break;
            case SDLK_BACKSPACE:
                action = Action::Backspace;
                break;
            case SDLK_ESCAPE:
                action = Action::Quit;
                break;
            case SDLK_F2:
            case SDLK_TAB:
                action = Action::NextGraphics;
                break;
            case SDLK_F3:
                action = Action::NextGame;
                break;
            case SDLK_F4:
                action = Action::Restart;
                break;
            case SDLK_F5:
                action = Action::Menu;
                break;
            case SDLK_F6:
                action = Action::Pause;
                break;
            case SDLK_F7:
                action = Action::Difficulty;
                break;
            default:
                break;
            }
            if (action != Action::None)
                result.push_back({action});
        }
    }
    return result;
}

} // namespace arcade
