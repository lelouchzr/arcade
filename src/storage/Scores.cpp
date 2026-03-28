#include "arcade/Scores.hpp"
#include "FileLock.hpp"
#include "ScoreFile.hpp"
#include <stdexcept>

namespace arcade {

Scores::Scores(std::filesystem::path path) : path_(std::filesystem::absolute(std::move(path))) {
    std::filesystem::create_directories(path_.parent_path());
    FileLock lock(path_.string() + ".lock", false);
    (void)storage::read(path_);
}

std::vector<Score> Scores::top(const std::string &game, std::size_t count) const {
    FileLock lock(path_.string() + ".lock", false);
    auto scores = storage::read(path_);
    storage::retainBest(scores);
    std::vector<Score> result;
    for (const auto &score : scores)
        if (score.game == game && result.size() < count)
            result.push_back(score);
    return result;
}

void Scores::add(std::string game, std::string player, std::int64_t points) {
    if (points <= 0)
        return;
    if (points > 1000000000000LL || game.empty() || game.size() > 48 || player.empty() ||
        player.size() > 16 ||
        game.find_first_not_of("abcdefghijklmnopqrstuvwxyz0123456789_-") != std::string::npos ||
        player.find_first_of("\r\n\0", 0, 3) != std::string::npos)
        throw std::invalid_argument("invalid score entry");
    // The sidecar inode is stable even while writeAtomic replaces the score file.
    FileLock lock(path_.string() + ".lock", true);
    auto scores = storage::read(path_);
    scores.push_back({std::move(game), std::move(player), points});
    storage::retainBest(scores);
    storage::writeAtomic(path_, scores);
}

} // namespace arcade
