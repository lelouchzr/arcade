#pragma once
#include "arcade/Api.hpp"

namespace arcade::test {
class ScriptDisplay final : public IGraphics {
  public:
    void open() override;
    void close() noexcept override;
    std::vector<Input> poll() override;
    void draw(const Frame &frame) override;

  private:
    static void log(const std::string &line);
};
} // namespace arcade::test
