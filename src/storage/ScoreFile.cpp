#include "ScoreFile.hpp"
#include <algorithm>
#include <cerrno>
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <map>
#include <sstream>
#include <stdexcept>
#include <system_error>
#include <unistd.h>

namespace arcade::storage {

std::vector<Score> read(const std::filesystem::path &path) {
    if (!std::filesystem::exists(path))
        return {};
    std::ifstream file(path);
    if (!file)
        throw std::runtime_error("cannot read score file '" + path.string() + "'");
    std::vector<Score> scores;
    std::string line;
    while (scores.size() < 10000 && std::getline(file, line)) {
        Score score;
        std::istringstream row(line);
        if (row >> std::quoted(score.game) >> std::quoted(score.player) >> score.points) {
            row >> std::ws;
            if (row.eof() && score.points >= 0 && score.points <= 1000000000000LL &&
                !score.game.empty() && score.game.size() <= 48 && !score.player.empty() &&
                score.player.size() <= 16)
                scores.push_back(std::move(score));
        }
    }
    if (file.bad())
        throw std::runtime_error("error reading score file");
    return scores;
}

void retainBest(std::vector<Score> &scores) {
    std::stable_sort(scores.begin(), scores.end(),
                     [](const Score &a, const Score &b) { return a.points > b.points; });
    std::map<std::string, int> counts;
    std::erase_if(scores, [&](const Score &score) { return ++counts[score.game] > 10; });
}

void writeAtomic(const std::filesystem::path &path, const std::vector<Score> &scores) {
    std::ostringstream content;
    for (const auto &score : scores)
        content << std::quoted(score.game) << ' ' << std::quoted(score.player) << ' '
                << score.points << '\n';
    auto pattern = path.string() + ".tmp.XXXXXX";
    std::vector<char> name(pattern.begin(), pattern.end());
    name.push_back('\0');
    int descriptor = ::mkstemp(name.data());
    if (descriptor < 0)
        throw std::system_error(errno, std::generic_category(), "cannot create score transaction");
    const std::filesystem::path temporary(name.data());
    try {
        const auto bytes = content.str();
        std::size_t offset = 0;
        while (offset < bytes.size()) {
            const auto count = ::write(descriptor, bytes.data() + offset, bytes.size() - offset);
            if (count < 0 && errno == EINTR)
                continue;
            if (count <= 0)
                throw std::system_error(errno, std::generic_category(), "cannot write scores");
            offset += static_cast<std::size_t>(count);
        }
        if (::fsync(descriptor) < 0)
            throw std::system_error(errno, std::generic_category(), "cannot sync scores");
        if (::close(descriptor) < 0) {
            descriptor = -1;
            throw std::system_error(errno, std::generic_category(), "cannot close scores");
        }
        descriptor = -1;
        std::filesystem::rename(temporary, path);
        const int directory = ::open(path.parent_path().c_str(), O_RDONLY | O_CLOEXEC);
        if (directory >= 0) {
            const int result = ::fsync(directory);
            const int saved = errno;
            ::close(directory);
            if (result < 0 && saved != EINVAL && saved != ENOTSUP)
                throw std::system_error(saved, std::generic_category(),
                                        "cannot sync score directory");
        }
    } catch (...) {
        if (descriptor >= 0)
            ::close(descriptor);
        std::error_code ignored;
        std::filesystem::remove(temporary, ignored);
        throw;
    }
}

} // namespace arcade::storage
