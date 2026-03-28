#pragma once

#include <filesystem>

namespace arcade {

class FileLock {
  public:
    FileLock(const std::filesystem::path &path, bool exclusive);
    ~FileLock();
    FileLock(const FileLock &) = delete;
    FileLock &operator=(const FileLock &) = delete;

  private:
    int descriptor_ = -1;
};

} // namespace arcade
