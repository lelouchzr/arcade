#pragma once

#include "arcade/Api.hpp"
#include <cstdint>
#include <string_view>

namespace arcade::benchmark {

class Digest {
  public:
    void add(std::uint64_t value);
    void add(std::string_view value);
    void add(const Snapshot &snapshot);
    void add(const Frame &frame);
    [[nodiscard]] std::uint64_t value() const;

  private:
    std::uint64_t value_ = 14695981039346656037ULL;
};

} // namespace arcade::benchmark
