#pragma once

#include <filesystem>
#include <string>

namespace arcade::benchmark {

class Workspace {
  public:
    Workspace(const std::filesystem::path &modules, const std::filesystem::path &graphics);
    ~Workspace();
    Workspace(const Workspace &) = delete;
    Workspace &operator=(const Workspace &) = delete;
    [[nodiscard]] const std::filesystem::path &root() const;

  private:
    std::filesystem::path previous_;
    std::filesystem::path root_;
};

} // namespace arcade::benchmark
