#pragma once

#include "arcade/Api.hpp"
#include <cstdint>

namespace arcade::benchmark {

class Headless final : public IGraphics {
  public:
    void open() override;
    void close() noexcept override;
    std::vector<Input> poll() override;
    void draw(const Frame &frame) override;
    void reset();
    [[nodiscard]] std::uint64_t checksum() const;
    [[nodiscard]] std::uint64_t commands() const;

  private:
    std::uint64_t checksum_ = 0;
    std::uint64_t commands_ = 0;
    void quad(const Quad &value);
};

} // namespace arcade::benchmark
