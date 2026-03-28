#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

// This SDK is deliberately independent of every graphics library.
namespace arcade {
inline constexpr std::uint32_t AbiVersion = 3;
enum class PluginKind : std::uint32_t { Game = 1, Graphics = 2 };
enum class Action {
    None,
    Up,
    Down,
    Left,
    Right,
    Primary,
    Secondary,
    Confirm,
    Backspace,
    Text,
    NextGraphics,
    NextGame,
    Restart,
    Menu,
    Quit,
    Pause,
    Difficulty,
    Stop,
    Jump,
    Camera,
    PlayerTwoUp,
    PlayerTwoDown,
    PlayerTwoLeft,
    PlayerTwoRight,
    PlayerTwoFire
};

struct Input {
    Action action = Action::None;
    char text = 0;
};

struct Point {
    int x = 0;
    int y = 0;
    bool operator==(const Point &) const = default;
};
enum class Cell {
    Empty,
    Wall,
    Head,
    Body,
    Food,
    Hidden,
    Flag,
    Mine,
    Zero,
    One,
    Two,
    Three,
    Four,
    Five,
    Six,
    Seven,
    Eight,
    Paddle,
    Ball,
    Brick,
    Player,
    Pellet,
    PowerPellet,
    GhostRed,
    GhostPink,
    GhostCyan,
    GhostOrange,
    FrightenedGhost,
    GhostEyes,
    Home,
    Trail,
    Claimed,
    Boundary,
    Qix,
    Spark,
    Fire,
    Projectile,
    EnemyProjectile,
    CentipedeHead,
    CentipedeBody,
    Obstacle,
    Ship,
    EnemyGun,
    Powerup,
    BadPowerup,
    BonusFood,
    Question,
    JumpingPlayer,
    JumpingGhost,
    SecondPlayer,
    SlowProjectile,
    FastProjectile
};
enum class Status { Running, Won, Lost };

struct Camera {
    Point center;
    int columns = 17;
    int rows = 13;
    bool isometric = true;
};

struct Sprite {
    Point position;
    Cell cell = Cell::Empty;
    double elevation = 0; // Height above the board, measured in cell units.
};

struct Snapshot {
    int width = 0, height = 0;
    std::vector<Cell> cells;
    Point cursor{-1, -1};
    std::int64_t score = 0;
    std::uint64_t generation = 0; // Advance whenever a game starts a fresh score round.
    double secondsLeft = -1;
    Status status = Status::Running;
    std::vector<std::string> information;
    std::optional<Camera> camera;
    std::vector<Sprite> sprites;
};

struct Color {
    std::uint8_t r, g, b;
};

struct Quad {
    int x, y, width, height;
    Color color;
    bool outline = false;
};

struct Label {
    int x, y, scale;
    std::string text;
    Color color;
};

struct Frame {
    static constexpr int Width = 1280, Height = 800;
    std::vector<Quad> quads;
    std::vector<Label> labels;
    std::optional<Snapshot> board; // Semantic layout for compact terminal displays.
    std::string title;
    std::string player;
};

class IPlugin {
  public:
    virtual ~IPlugin() = default;
};

class IGame : public IPlugin {
  public:
    // reset(seed) makes simulations reproducible and never opens a window.
    virtual void reset(std::uint64_t seed) = 0;
    virtual void update(double seconds, const std::vector<Input> &inputs) = 0;
    [[nodiscard]] virtual Snapshot snapshot() const = 0;
};

class IGraphics : public IPlugin {
  public:
    virtual void open() = 0;
    virtual void close() noexcept = 0;
    [[nodiscard]] virtual std::vector<Input> poll() = 0;
    virtual void draw(const Frame &frame) = 0;
};

// All modules export exactly the same entry point and descriptor shape.
struct PluginDescriptor {
    std::uint32_t abi;
    PluginKind kind;
    const char *id;
    const char *name;
    IPlugin *(*create)();
    void (*destroy)(IPlugin *) noexcept;
};

using EntryPoint = const PluginDescriptor *(*)();
} // namespace arcade
#if defined(_WIN32)
#define ARCADE_EXPORT __declspec(dllexport)
#else
#define ARCADE_EXPORT __attribute__((visibility("default")))
#endif
#define ARCADE_PLUGIN(TYPE, KIND, ID, NAME)                                                        \
    extern "C" ARCADE_EXPORT const arcade::PluginDescriptor *arcade_plugin() {                     \
        static const arcade::PluginDescriptor descriptor{                                          \
            arcade::AbiVersion,                                                                    \
            KIND,                                                                                  \
            ID,                                                                                    \
            NAME,                                                                                  \
            []() -> arcade::IPlugin * { return new TYPE(); },                                      \
            [](arcade::IPlugin *value) noexcept { delete value; }};                                \
        return &descriptor;                                                                        \
    }
