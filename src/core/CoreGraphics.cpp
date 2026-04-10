#include "Core.hpp"
#include <utility>

namespace arcade {

void Core::switchGraphics(std::size_t index) {
    std::unique_ptr<Plugin> replacement;
    bool previousClosed = false;
    try {
        replacement = std::make_unique<Plugin>(graphics_.at(index).path);
        auto &next = replacement->instance<IGraphics>();
        // Keep the previous instance alive until the replacement has opened successfully.
        if (display_) {
            display_->instance<IGraphics>().close();
            previousClosed = true;
        }
        next.open();
    } catch (const std::exception &error) {
        if (!display_)
            throw; // An explicitly requested startup backend must still report an error.
        const auto warning =
            "DISPLAY UNAVAILABLE: " + graphics_.at(index).id + " - " + error.what();
        replacement.reset();
        if (previousClosed)
            display_->instance<IGraphics>().open(); // A failed rollback remains fatal.
        unavailableGraphics_.insert(index);
        graphicsWarning_ = warning;
        return;
    }
    display_ = std::move(replacement);
    selectedGraphics_ = index;
    graphicsWarning_.clear();
}

void Core::cycleGraphics(bool previous) {
    auto index = selectedGraphics_;
    // Never retry the active backend or a failed backend during this session.
    for (std::size_t attempt = 1; attempt < graphics_.size(); ++attempt) {
        index = previous ? (index + graphics_.size() - 1) % graphics_.size()
                         : (index + 1) % graphics_.size();
        if (!unavailableGraphics_.contains(index)) {
            switchGraphics(index);
            return;
        }
    }
}

} // namespace arcade
