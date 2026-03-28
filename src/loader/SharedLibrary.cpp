#include "arcade/Plugin.hpp"
#include <dlfcn.h>

namespace arcade {

SharedLibrary::SharedLibrary(const std::filesystem::path &path) {
    dlerror();
    handle_ = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (!handle_) {
        const char *error = dlerror();
        throw std::runtime_error("cannot load '" + path.string() +
                                 "': " + (error ? error : "unknown loader error"));
    }
}

SharedLibrary::~SharedLibrary() {
    if (handle_)
        dlclose(handle_);
}

void *SharedLibrary::findSymbol(const char *name) const {
    dlerror();
    void *symbol = dlsym(handle_, name);
    if (const char *error = dlerror())
        throw std::runtime_error(error);
    if (!symbol)
        throw std::runtime_error("null plugin entry point");
    return symbol;
}

} // namespace arcade
