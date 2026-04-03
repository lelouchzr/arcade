#include "MineDifficulties.hpp"
#include "Minesweeper.hpp"
#include <algorithm>
#include <cmath>

namespace arcade {
bool Minesweeper::updateBonuses(double seconds, const std::vector<Input> &inputs) {
    animation_ = std::fmod(animation_ + seconds, 60.0);
    scared_ = std::max(0.0, scared_ - seconds);
    for (const auto &input : inputs) {
        if (input.action == Action::Difficulty) {
            if (editing_) {
                difficulty_ = 0;
                reset(seed_);
            } else {
                difficulty_ = (difficulty_ + 1) % 4;
                reset(seed_);
                if (difficulty_ == 3) {
                    editing_ = true;
                    editor_.begin(custom_);
                }
            }
            return true;
        }
        if (!editing_)
            continue;
        if (input.action == Action::Backspace) {
            difficulty_ = 0;
            reset(seed_);
            return true;
        }
        if (input.action == Action::Confirm) {
            custom_ = editor_.configuration();
            reset(seed_);
            return true;
        }
        editor_.handle(input);
    }
    return editing_;
}

void Minesweeper::mark(std::size_t cell, bool cycle) {
    if (!cycle) {
        flags_[cell] = !flags_[cell];
        questions_[cell] = false;
    } else if (flags_[cell]) {
        flags_[cell] = false;
        questions_[cell] = true;
    } else if (questions_[cell])
        questions_[cell] = false;
    else
        flags_[cell] = true;
}

void Minesweeper::decorate(Snapshot &view) const {
    if (width_ > 30 || height_ > 21)
        view.camera = Camera{cursor_, std::min(width_, 30), std::min(height_, 21), false};
    const std::string name =
        difficulty_ < 3 ? bonus::MineDifficulties[static_cast<std::size_t>(difficulty_)].name
                        : "CUSTOM";
    const std::string mines = view.information[3];
    view.information = {"",    "ARROWS MOVE SPACE SHOW", "F FLAG  ? FLAG/?/CLEAR",
                        mines, "FIRST REVEAL SAFE",      "D/F7  " + name};
    if (editing_) {
        view.information = editor_.information();
        return;
    }
    std::string face = "FACE  [ :-) ]";
    if (status_ == Status::Won)
        face = "FACE  [ B-) ]  VICTORY";
    else if (status_ == Status::Lost)
        face = "FACE [ X-( ]  GAME OVER";
    else if (scared_ > 0)
        face = "FACE  [ :O ]  CAREFUL!";
    view.information.front() = face;
    if (status_ == Status::Running)
        return;
    // Presentation-only particles keep moving after the score and board are frozen.
    const int frame = static_cast<int>(animation_ * 8);
    view.information[4] = status_ == Status::Won
                              ? (frame % 2 ? "*** CLEARED! ***" : "+++ CLEARED! +++")
                              : (frame % 2 ? "BOOM!  * * *" : "BOOM!   ***");
    for (int i = 0; i < 6; ++i) {
        const int x = (i * 7 + frame / 2) % width_;
        const int y = (i * 3 + frame) % height_;
        view.sprites.push_back(
            {{x, y}, status_ == Status::Won ? Cell::BonusFood : Cell::Mine, 0.15 + 0.1 * (i % 3)});
    }
}
} // namespace arcade
