#pragma once
#include <array>
#include <csignal>

namespace arcade {
// Terminal libraries install process-wide callbacks; none may survive their module unload.
class SignalState {
  public:
    SignalState() = default;
    ~SignalState();
    SignalState(const SignalState &) = delete;
    SignalState &operator=(const SignalState &) = delete;
    void capture();
    void restore() noexcept;

  private:
    static constexpr std::array<int, 8> signals_{SIGINT,  SIGQUIT, SIGTERM, SIGWINCH,
                                                 SIGTSTP, SIGCONT, SIGHUP,  SIGPIPE};
    std::array<struct sigaction, signals_.size()> previous_{};
    std::array<bool, signals_.size()> saved_{};
};
} // namespace arcade
