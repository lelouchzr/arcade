#pragma once
#include "MineConfiguration.hpp"
#include "arcade/Api.hpp"
#include <string>
#include <vector>

namespace arcade::bonus {
class MineEditor {
  public:
    void begin(MineConfiguration configuration);
    void handle(const Input &input);
    [[nodiscard]] MineConfiguration configuration() const;
    [[nodiscard]] std::vector<std::string> information() const;

  private:
    MineConfiguration configuration_;
    int selected_ = 0;
    std::string digits_;
    void adjust(int delta);
    void assign(int value);
    void clamp();
};
} // namespace arcade::bonus
