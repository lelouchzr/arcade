#pragma once

#include "../../src/core/Core.hpp"
#include <string>

namespace arcade::benchmark {

class CoreAccess {
  public:
    static void variant(Core &core, int index);
    static void prepare(Core &core, const std::string &game, std::uint64_t seed);
};

} // namespace arcade::benchmark
