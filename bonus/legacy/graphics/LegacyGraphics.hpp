#pragma once

#include "../sdk/IGraphic.hpp"
#include "arcade/Api.hpp"
#include "arcade/Plugin.hpp"
#include <memory>

namespace arcade::legacy {

enum class LegacyProfile { Terminal, RawText, Sdl, Sfml };

class LegacyGraphics final : public IGraphics {
  public:
    LegacyGraphics();
    explicit LegacyGraphics(std::filesystem::path path);
    ~LegacyGraphics() override;
    void open() override;
    void close() noexcept override;
    [[nodiscard]] std::vector<Input> poll() override;
    void draw(const Frame &frame) override;

  private:
    std::filesystem::path path_;
    std::unique_ptr<SharedLibrary> library_;
    std::unique_ptr<Arcade::SharedInterfaces::IGraphic> display_;
    LegacyProfile profile_ = LegacyProfile::Terminal;
    bool playing_ = false;
    std::string player_ = "PLAYER";
    std::size_t letter_ = 0;
    static std::filesystem::path configuredPath();
    static LegacyProfile configuredProfile();
    static Arcade::Types::ForeColor color(Color ink);
    static std::string escapeFormat(const std::string &text);
    void text(const std::string &value, int x, int y);
    void board(const Snapshot &snapshot);
    void panels(const Frame &frame);
    void footer(const std::string &message);
    void editName(int change, std::vector<Input> &inputs);
};

} // namespace arcade::legacy
