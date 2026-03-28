#include "arcade/SignalState.hpp"
#include <stdexcept>

namespace arcade {
SignalState::~SignalState() {
    restore();
}

void SignalState::capture() {
    restore();
    for (std::size_t index = 0; index < signals_.size(); ++index) {
        if (sigaction(signals_[index], nullptr, &previous_[index]) != 0) {
            restore();
            throw std::runtime_error("cannot save terminal signal handlers");
        }
        saved_[index] = true;
    }
}

void SignalState::restore() noexcept {
    for (std::size_t index = 0; index < signals_.size(); ++index)
        if (saved_[index]) {
            sigaction(signals_[index], &previous_[index], nullptr);
            saved_[index] = false;
        }
}
} // namespace arcade
