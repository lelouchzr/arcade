#include "arcade/Plugin.hpp"

namespace arcade {

Plugin::Plugin(const std::filesystem::path &path) : library_(path) {
    descriptor_ = library_.symbol<EntryPoint>("arcade_plugin")();
    if (!descriptor_ || descriptor_->abi != AbiVersion || !descriptor_->id || !descriptor_->name ||
        !descriptor_->create || !descriptor_->destroy ||
        (descriptor_->kind != PluginKind::Game && descriptor_->kind != PluginKind::Graphics))
        throw std::runtime_error("incompatible Arcade ABI in '" + path.string() + "'");
    const std::string id(descriptor_->id);
    if (id.empty() || id.size() > 48 ||
        id.find_first_not_of("abcdefghijklmnopqrstuvwxyz0123456789_-") != std::string::npos)
        throw std::runtime_error("invalid plugin identifier in '" + path.string() + "'");
    info_ = {std::filesystem::weakly_canonical(path), descriptor_->kind, id, descriptor_->name};
}

Plugin::~Plugin() {
    if (instance_)
        descriptor_->destroy(instance_);
}

const PluginInfo &Plugin::info() const {
    return info_;
}

IPlugin &Plugin::ensureInstance() {
    if (!instance_)
        instance_ = descriptor_->create();
    if (!instance_)
        throw std::runtime_error("plugin factory returned null");
    return *instance_;
}

} // namespace arcade
