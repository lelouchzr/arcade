#include "Minesweeper.hpp"
#ifdef ARCADE_BONUS
#include "MineDifficulties.hpp"
#endif
#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>

namespace arcade {

Minesweeper::Minesweeper() {
    reset(1);
}

int Minesweeper::index(Point point) const {
    return point.y * width_ + point.x;
}

void Minesweeper::reset(std::uint64_t seed) {
    seed_ = seed;
    random_.seed(seed);
    ++generation_;
#ifdef ARCADE_BONUS
    if (difficulty_ < 3) {
        const auto &difficulty = bonus::MineDifficulties[static_cast<std::size_t>(difficulty_)];
        width_ = difficulty.width;
        height_ = difficulty.height;
        mineCount_ = difficulty.mines;
        seconds_ = difficulty.seconds;
    } else {
        width_ = custom_.width;
        height_ = custom_.height;
        mineCount_ = custom_.mines;
        seconds_ = custom_.seconds;
    }
    editing_ = false;
    animation_ = scared_ = 0;
#else
    width_ = height_ = 9;
    mineCount_ = 10;
    seconds_ = 180;
#endif
    const auto size = static_cast<std::size_t>(width_ * height_);
    mines_.assign(size, false);
    revealed_.assign(size, false);
    flags_.assign(size, false);
#ifdef ARCADE_BONUS
    questions_.assign(size, false);
#endif
    adjacent_.assign(size, 0);
    cursor_ = {width_ / 2, height_ / 2};
    planted_ = false;
    score_ = 0;
    status_ = Status::Running;
}

std::vector<int> Minesweeper::neighbours(int cell) const {
    std::vector<int> result;
    const int x = cell % width_, y = cell / width_;
    for (int dy = -1; dy <= 1; ++dy)
        for (int dx = -1; dx <= 1; ++dx)
            if ((dx || dy) && x + dx >= 0 && y + dy >= 0 && x + dx < width_ && y + dy < height_)
                result.push_back((y + dy) * width_ + x + dx);
    return result;
}

void Minesweeper::plant(int safeCell) {
    auto safe = neighbours(safeCell);
    safe.push_back(safeCell);
    std::vector<int> candidates;
    for (int i = 0; i < width_ * height_; ++i)
        if (std::find(safe.begin(), safe.end(), i) == safe.end())
            candidates.push_back(i);
    std::shuffle(candidates.begin(), candidates.end(), random_);
    for (int i = 0; i < mineCount_; ++i)
        mines_[static_cast<std::size_t>(candidates[static_cast<std::size_t>(i)])] = true;
    for (int i = 0; i < width_ * height_; ++i)
        for (int neighbour : neighbours(i))
            if (mines_[static_cast<std::size_t>(neighbour)])
                ++adjacent_[static_cast<std::size_t>(i)];
    planted_ = true;
}

void Minesweeper::reveal(int cell) {
    if (flags_[static_cast<std::size_t>(cell)] || revealed_[static_cast<std::size_t>(cell)])
        return;
    if (!planted_)
        plant(cell); // A whole 3x3 neighbourhood is protected: first reveal is always zero.
    if (mines_[static_cast<std::size_t>(cell)]) {
        status_ = Status::Lost;
        return;
    }
    std::vector<int> pending{cell};
    while (!pending.empty()) {
        const auto current = static_cast<std::size_t>(pending.back());
        pending.pop_back();
        if (revealed_[current] || flags_[current] || mines_[current])
            continue;
        revealed_[current] = true;
#ifdef ARCADE_BONUS
        questions_[current] = false;
#endif
        score_ += 10;
        if (!adjacent_[current]) {
            const auto around = neighbours(static_cast<int>(current));
            pending.insert(pending.end(), around.begin(), around.end());
        }
    }
    const auto revealed = std::count(revealed_.begin(), revealed_.end(), true);
    if (revealed == width_ * height_ - mineCount_) {
        status_ = Status::Won;
        score_ += static_cast<std::int64_t>(std::ceil(seconds_));
    }
}

void Minesweeper::update(double seconds, const std::vector<Input> &inputs) {
    if (!std::isfinite(seconds) || seconds < 0 || seconds > 60)
        throw std::invalid_argument("invalid game delta time");
#ifdef ARCADE_BONUS
    if (updateBonuses(seconds, inputs))
        return;
#endif
    if (status_ != Status::Running)
        return;
    // The countdown starts with the first reveal, not while reading the board.
    if (planted_) {
        seconds_ = std::max(0.0, seconds_ - seconds);
        if (seconds_ <= 0) {
            status_ = Status::Lost;
            return;
        }
    }
    for (const auto &input : inputs) {
        switch (input.action) {
        case Action::Up:
            cursor_.y = std::max(0, cursor_.y - 1);
            break;
        case Action::Down:
            cursor_.y = std::min(height_ - 1, cursor_.y + 1);
            break;
        case Action::Left:
            cursor_.x = std::max(0, cursor_.x - 1);
            break;
        case Action::Right:
            cursor_.x = std::min(width_ - 1, cursor_.x + 1);
            break;
        case Action::Primary:
        case Action::Confirm:
#ifdef ARCADE_BONUS
            scared_ = 0.3;
#endif
            reveal(index(cursor_));
            break;
        case Action::Secondary: {
            const auto cell = static_cast<std::size_t>(index(cursor_));
            if (!revealed_[cell]) {
#ifdef ARCADE_BONUS
                mark(cell, input.text == '?');
#else
                flags_[cell] = !flags_[cell];
#endif
            }
            break;
        }
        default:
            break;
        }
        if (status_ != Status::Running)
            break;
    }
}

Snapshot Minesweeper::snapshot() const {
    Snapshot view;
    view.width = width_;
    view.height = height_;
    view.cursor = cursor_;
    view.score = score_;
    view.generation = generation_;
    view.secondsLeft = seconds_;
    view.status = status_;
    for (int i = 0; i < width_ * height_; ++i) {
        const auto cell = static_cast<std::size_t>(i);
        if (status_ != Status::Running && mines_[cell])
            view.cells.push_back(Cell::Mine);
        else if (revealed_[cell])
            view.cells.push_back(static_cast<Cell>(static_cast<int>(Cell::Zero) + adjacent_[cell]));
        else {
            Cell hidden = flags_[cell] ? Cell::Flag : Cell::Hidden;
#ifdef ARCADE_BONUS
            if (questions_[cell])
                hidden = Cell::Question;
#endif
            view.cells.push_back(hidden);
        }
    }
    const auto flags = std::count(flags_.begin(), flags_.end(), true);
    view.information = {"ARROWS   MOVE CURSOR", "SPACE    REVEAL", "F        TOGGLE FLAG",
                        "MINES    " + std::to_string(mineCount_ - flags), "FIRST REVEAL IS SAFE"};
#ifdef ARCADE_BONUS
    decorate(view);
#endif
    return view;
}
} // namespace arcade
#ifndef ARCADE_NO_ENTRY
ARCADE_PLUGIN(arcade::Minesweeper, arcade::PluginKind::Game, "minesweeper", "MINESWEEPER")
#endif
