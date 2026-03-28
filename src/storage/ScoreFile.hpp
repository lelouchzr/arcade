#pragma once

#include "arcade/Scores.hpp"

namespace arcade::storage {

std::vector<Score> read(const std::filesystem::path &path);
void writeAtomic(const std::filesystem::path &path, const std::vector<Score> &scores);
void retainBest(std::vector<Score> &scores);

} // namespace arcade::storage
