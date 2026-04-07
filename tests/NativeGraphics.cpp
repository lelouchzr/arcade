#include "BoardRenderer.hpp"
#include "arcade/Plugin.hpp"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>
#include <X11/keysym.h>
#undef None
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>

namespace {
using namespace arcade;

void require(bool condition, const std::string &message) {
    if (!condition)
        throw std::runtime_error(message);
}

Window findWindow(Display *display, Window root) {
    XWindowAttributes candidate{};
    if (!XGetWindowAttributes(display, root, &candidate) || candidate.map_state != IsViewable)
        return 0;
    char *name = nullptr;
    if (XFetchName(display, root, &name) && name) {
        const bool found = std::string(name).find("ARCADE / 400") != std::string::npos;
        XFree(name);
        if (found)
            return root;
    }
    for (const char *property : {"_NET_WM_NAME", "WM_NAME"}) {
        Atom actualType = 0;
        int format = 0;
        unsigned long countBytes = 0, remaining = 0;
        unsigned char *utf8 = nullptr;
        XGetWindowProperty(display, root, XInternAtom(display, property, False), 0, 256, False,
                           AnyPropertyType, &actualType, &format, &countBytes, &remaining, &utf8);
        if (utf8) {
            const bool found =
                std::string(reinterpret_cast<char *>(utf8), countBytes).find("ARCADE / 400") !=
                std::string::npos;
            XFree(utf8);
            if (found)
                return root;
        }
    }
    if (root != DefaultRootWindow(display) && XGetWindowAttributes(display, root, &candidate) &&
        candidate.map_state == IsViewable && candidate.width == Frame::Width &&
        candidate.height == Frame::Height)
        return root;
    Window base = 0, parent = 0, *children = nullptr;
    unsigned count = 0;
    if (!XQueryTree(display, root, &base, &parent, &children, &count))
        return 0;
    Window result = 0;
    for (unsigned i = 0; i < count && !result; ++i)
        result = findWindow(display, children[i]);
    if (children)
        XFree(children);
    return result;
}

Frame markers() {
    Frame frame;
    frame.title = "ARCADE NATIVE";
    frame.quads = {{0, 0, 1280, 800, {10, 15, 23}, false},
                   {32, 32, 128, 96, {255, 0, 0}, false},
                   {192, 32, 128, 96, {0, 255, 0}, false},
                   {352, 32, 128, 96, {0, 0, 255}, false},
                   {512, 32, 128, 96, {255, 255, 255}, true}};
    frame.labels.push_back({32, 160, 4, "ARCADE NATIVE / 0123456789", {255, 255, 255}});
    return frame;
}

void pixelCheck(Display *display, Window window, const std::string &id) {
    XSync(display, False);
    XWindowAttributes attributes{};
    XGetWindowAttributes(display, window, &attributes);
    if (attributes.width != Frame::Width || attributes.height != Frame::Height)
        throw std::runtime_error(id + ": wrong window size");
    XImage *image =
        XGetImage(display, window, 0, 0, Frame::Width, Frame::Height, AllPlanes, ZPixmap);
    require(image != nullptr, id + ": cannot read native framebuffer");
    auto pixel = [&](int x, int y) {
        return static_cast<unsigned>(XGetPixel(image, x, y)) & 0xffffffU;
    };
    const bool good = pixel(80, 80) == 0xff0000U && pixel(240, 80) == 0x00ff00U &&
                      pixel(400, 80) == 0x0000ffU && pixel(550, 80) == 0x0a0f17U;
    if (const char *directory = std::getenv("ARCADE_NATIVE_CAPTURES")) {
        std::ofstream output(std::filesystem::path(directory) / (id + ".ppm"), std::ios::binary);
        output << "P6\n1280 800\n255\n";
        for (int y = 0; y < Frame::Height; ++y)
            for (int x = 0; x < Frame::Width; ++x) {
                auto p = pixel(x, y);
                for (unsigned shift : {16U, 8U, 0U})
                    output.put(static_cast<char>((p >> shift) & 255U));
            }
        require(static_cast<bool>(output), "cannot write native capture");
    }
    if (!good)
        std::cerr << id << " native pixels " << std::hex << pixel(80, 80) << ' ' << pixel(240, 80)
                  << ' ' << pixel(400, 80) << ' ' << pixel(550, 80) << std::dec << '\n';
    XDestroyImage(image);
    require(
        good,
        id + ": RGB order, presentation, rectangle or outline differs from expected native pixels");
}

void resizeCheck(Display *display, Window window, IGraphics &graphics, const std::string &id) {
    XResizeWindow(display, window, 1000, 800);
    XSync(display, False);
    for (int i = 0; i < 10; ++i) {
        (void)graphics.poll();
        graphics.draw(markers());
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    XSync(display, False);
    XImage *image = XGetImage(display, window, 0, 0, 1000, 800, AllPlanes, ZPixmap);
    require(image != nullptr, id + ": cannot read resized framebuffer");
    auto pixel = [&](int x, int y) { return XGetPixel(image, x, y) & 0xffffffUL; };
    const bool good = pixel(62, 150) == 0xff0000UL && pixel(187, 150) == 0x00ff00UL &&
                      pixel(312, 150) == 0x0000ffUL && pixel(62, 20) == 0x0a0f17UL;
    XDestroyImage(image);
    require(good, id + ": resizing lost colors or aspect ratio");
    XResizeWindow(display, window, Frame::Width, Frame::Height);
    XSync(display, False);
    (void)graphics.poll();
}

void keyCheck(Display *display, Window window, IGraphics &graphics, const std::string &id) {
    XSetInputFocus(display, window, RevertToParent, CurrentTime);
    XSync(display, False);
    for (auto symbol : {XK_Up, XK_F7, XK_Tab, XK_A}) {
        const auto code = XKeysymToKeycode(display, static_cast<KeySym>(symbol));
        XTestFakeKeyEvent(display, code, True, CurrentTime);
        XTestFakeKeyEvent(display, code, False, CurrentTime);
    }
    XFlush(display);
    std::vector<Input> inputs;
    for (int i = 0; i < 12; ++i) {
        auto batch = graphics.poll();
        inputs.insert(inputs.end(), batch.begin(), batch.end());
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    for (auto action : {Action::Up, Action::Difficulty, Action::NextGraphics, Action::Text})
        require(std::any_of(inputs.begin(), inputs.end(),
                            [&](auto input) { return input.action == action; }),
                id +
                    ": native keyboard event missing: " + std::to_string(static_cast<int>(action)));
}
} // namespace

int main(int argc, char **argv) {
    try {
        require(argc >= 3, "expected library directory and display names");
        auto *display = XOpenDisplay(nullptr);
        require(display != nullptr, "test requires X11");
        std::unique_ptr<Plugin> previous;
        std::size_t rendered = 0;
        for (int round = 0; round < 2; ++round)
            for (int index = 2; index < argc; ++index) {
                std::string name = argv[index];
                auto replacement = std::make_unique<Plugin>(std::filesystem::path(argv[1]) /
                                                            ("arcade_" + name + ".so"));
                if (previous)
                    previous->instance<IGraphics>().close();
                auto &graphics = replacement->instance<IGraphics>();
                graphics.open();
                // Match Core's ownership order: the old module unloads after the new one opens.
                previous = std::move(replacement);
                for (int i = 0; i < 5; ++i) {
                    graphics.draw(markers());
                    (void)graphics.poll();
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
                const bool terminal =
                    name == "ndk++" || name == "aalib" || name == "libcaca" || name == "ncurses";
                if (terminal && std::getenv("ARCADE_TEST_TERMINAL_KEYS")) {
                    std::cout << "TERMINAL_READY" << std::endl;
                    bool received = false;
                    for (int attempt = 0; attempt < 100 && !received; ++attempt) {
                        auto inputs = graphics.poll();
                        received = std::any_of(inputs.begin(), inputs.end(), [](auto input) {
                            return input.action == Action::Text && input.text == 'd';
                        });
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    }
                    require(received, name + ": terminal keyboard input missing");
                }
                if (!terminal) {
                    auto window = findWindow(display, DefaultRootWindow(display));
                    require(window != 0, name + ": no native window");
                    // XGetImage requires the entire drawable on screen (SFML can start partly
                    // offscreen).
                    XMoveWindow(display, window, 0, 0);
                    XSync(display, False);
                    graphics.draw(markers());
                    (void)graphics.poll();
                    pixelCheck(display, window, name);
                    keyCheck(display, window, graphics, name);
                    resizeCheck(display, window, graphics, name);
                }
                for (const auto &game : {"snake", "minesweeper", "nibbler", "pacman", "qix",
                                         "centipede", "solarfox"}) {
                    auto path =
                        std::filesystem::path(argv[1]) / (std::string("arcade_") + game + ".so");
                    if (!std::filesystem::exists(path))
                        continue;
                    Plugin plugin(path);
                    auto &model = plugin.instance<IGame>();
                    model.reset(400);
                    for (bool bonus : {false, true}) {
                        model.update(.05, bonus ? std::vector<Input>{{Action::Difficulty}}
                                                : std::vector<Input>{});
                        auto snapshot = model.snapshot();
                        Frame frame;
                        frame.title = plugin.info().name;
                        frame.board = snapshot;
                        BoardRenderer::draw(frame, snapshot);
                        graphics.draw(frame);
                        (void)graphics.poll();
                        ++rendered;
                    }
                }
                std::cout << name << ": presentation, events and seven games passed; round "
                          << round + 1 << std::endl;
            }
        if (previous)
            previous->instance<IGraphics>().close();
        previous.reset();
        XCloseDisplay(display);
        std::cout << rendered
                  << " real game/display frames; hot replacement and unloading passed\n";
        return 0;
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
