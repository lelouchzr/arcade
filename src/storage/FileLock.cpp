#include "FileLock.hpp"
#include <cerrno>
#include <fcntl.h>
#include <stdexcept>
#include <sys/file.h>
#include <system_error>
#include <unistd.h>

namespace arcade {

FileLock::FileLock(const std::filesystem::path &path, bool exclusive) {
    descriptor_ = ::open(path.c_str(), O_CREAT | O_RDWR | O_CLOEXEC, 0600);
    if (descriptor_ < 0)
        throw std::system_error(errno, std::generic_category(), "cannot open score lock");
    int result;
    do {
        result = ::flock(descriptor_, exclusive ? LOCK_EX : LOCK_SH);
    } while (result < 0 && errno == EINTR);
    if (result < 0) {
        const int saved = errno;
        ::close(descriptor_);
        throw std::system_error(saved, std::generic_category(), "cannot lock scores");
    }
}

FileLock::~FileLock() {
    if (descriptor_ >= 0) {
        ::flock(descriptor_, LOCK_UN);
        ::close(descriptor_);
    }
}

} // namespace arcade
