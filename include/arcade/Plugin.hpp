#pragma once

#include "Api.hpp"
#include <filesystem>
#include <stdexcept>

namespace arcade {

class SharedLibrary {
  public:
    explicit SharedLibrary(const std::filesystem::path &path);
    ~SharedLibrary();
    SharedLibrary(const SharedLibrary &) = delete;
    SharedLibrary &operator=(const SharedLibrary &) = delete;

    template <class T> T symbol(const char *name) const {
        return reinterpret_cast<T>(findSymbol(name));
    }

  private:
    void *handle_ = nullptr;
    void *findSymbol(const char *name) const;
};

struct PluginInfo {
    std::filesystem::path path;
    PluginKind kind;
    std::string id;
    std::string name;
};

class Plugin {
  public:
    explicit Plugin(const std::filesystem::path &path);
    ~Plugin();
    Plugin(const Plugin &) = delete;
    Plugin &operator=(const Plugin &) = delete;
    [[nodiscard]] const PluginInfo &info() const;

    template <class T> T &instance() {
        auto *value = dynamic_cast<T *>(&ensureInstance());
        if (!value)
            throw std::runtime_error("plugin descriptor and implementation disagree");
        return *value;
    }

  private:
    SharedLibrary library_;
    const PluginDescriptor *descriptor_ = nullptr;
    IPlugin *instance_ = nullptr;
    PluginInfo info_;
    IPlugin &ensureInstance();
};

} // namespace arcade
