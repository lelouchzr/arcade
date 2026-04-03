#include "arcade/Scores.hpp"
#include <array>
#include <cerrno>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <sys/wait.h>
#include <unistd.h>

namespace {

void require(bool condition, const char *message) {
    if (!condition)
        throw std::runtime_error(message);
}

} // namespace

int main() {
    std::array<char, 64> pattern{};
    const std::string prefix = "/tmp/arcade-score-concurrency-XXXXXX";
    std::copy(prefix.begin(), prefix.end(), pattern.begin());
    char *directory = ::mkdtemp(pattern.data());
    if (!directory)
        return 1;
    const std::filesystem::path root(directory);
    try {
        const auto path = root / "scores.tsv";
        arcade::Scores first(path), second(path);
        first.add("snake", "FIRST", 100);
        second.add("snake", "SECOND", 200);
        require(first.top("snake").size() == 2, "stale instance overwrote another process score");
        int gate[2];
        require(::pipe(gate) == 0, "cannot create process barrier");
        std::array<pid_t, 8> children{};
        for (std::size_t worker = 0; worker < children.size(); ++worker) {
            children[worker] = ::fork();
            require(children[worker] >= 0, "fork failed");
            if (children[worker] == 0) {
                ::close(gate[1]);
                char byte;
                ssize_t received;
                do {
                    received = ::read(gate[0], &byte, 1);
                } while (received < 0 && errno == EINTR);
                if (received != 1)
                    ::_exit(2);
                try {
                    arcade::Scores writer(path);
                    for (int round = 0; round < 6; ++round) {
                        writer.add("worker" + std::to_string(worker), "PLAYER", round + 1);
                        writer.add("shared", "P" + std::to_string(worker),
                                   static_cast<std::int64_t>(worker) * 100 + round + 1);
                    }
                    ::_exit(0);
                } catch (...) {
                    ::_exit(3);
                }
            }
        }
        ::close(gate[0]);
        require(::write(gate[1], "12345678", 8) == 8, "cannot release workers");
        ::close(gate[1]);
        for (pid_t child : children) {
            int status = 0;
            require(::waitpid(child, &status, 0) == child && WIFEXITED(status) &&
                        WEXITSTATUS(status) == 0,
                    "score writer process failed");
        }
        for (std::size_t worker = 0; worker < children.size(); ++worker)
            require(first.top("worker" + std::to_string(worker), 10).size() == 6,
                    "concurrent update was lost");
        const auto best = first.top("shared", 20);
        require(best.size() == 10 && best.front().points == 706 && best.back().points == 603,
                "top ten transaction merge incorrect");
        require(std::filesystem::exists(path.string() + ".lock"), "stable sidecar lock missing");
        for (const auto &entry : std::filesystem::directory_iterator(root))
            require(entry.path().filename().string().find(".tmp.") == std::string::npos,
                    "transaction leaked a temporary file");
        std::filesystem::remove_all(root);
        std::cout << "8 processes, 96 writes, fresh readers and stable top-ten merge passed\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        std::filesystem::remove_all(root);
        return 1;
    }
}
