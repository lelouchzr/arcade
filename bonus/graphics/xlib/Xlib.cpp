#include "Xlib.hpp"
#include "../common/NativeKeys.hpp"
#include <X11/Xutil.h>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

namespace arcade::bonus {
Xlib::~Xlib() {
    close();
}

void Xlib::open() {
    if (display_)
        return;
    display_ = XOpenDisplay(nullptr);
    if (!display_)
        throw std::runtime_error("Xlib: cannot open display");
    window_ = XCreateSimpleWindow(display_, DefaultRootWindow(display_), 0, 0, Frame::Width,
                                  Frame::Height, 0, 0, 0);
    if (!window_) {
        close();
        throw std::runtime_error("Xlib: cannot create window");
    }
    XStoreName(display_, window_, "ARCADE / 400 - Xlib");
    XSelectInput(display_, window_,
                 ExposureMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask);
    closeAtom_ = XInternAtom(display_, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display_, window_, &closeAtom_, 1);
    gc_ = XCreateGC(display_, window_, 0, nullptr);
    if (!gc_) {
        close();
        throw std::runtime_error("Xlib: cannot create drawing context");
    }
    XMapWindow(display_, window_);
    XFlush(display_);
}

void Xlib::close() noexcept {
    if (display_) {
        if (gc_)
            XFreeGC(display_, gc_);
        if (window_)
            XDestroyWindow(display_, window_);
        XCloseDisplay(display_);
    }
    display_ = nullptr;
    window_ = 0;
    gc_ = nullptr;
}

std::vector<Input> Xlib::poll() {
    std::vector<Input> inputs;
    while (display_ && XPending(display_)) {
        XEvent event{};
        XNextEvent(display_, &event);
        if (event.type == ClientMessage && static_cast<Atom>(event.xclient.data.l[0]) == closeAtom_)
            inputs.push_back({Action::Quit});
        if (event.type == KeyPress || event.type == KeyRelease) {
            auto key = XLookupKeysym(&event.xkey, (event.xkey.state & ShiftMask) ? 1 : 0);
            auto input = nativeKey(static_cast<unsigned>(key), event.type == KeyPress);
            if (input.action != Action::None)
                inputs.push_back(input);
        }
    }
    return inputs;
}

void Xlib::draw(const Frame &frame) {
    if (!display_)
        throw std::runtime_error("Xlib display is closed");
    XWindowAttributes attrs{};
    XGetWindowAttributes(display_, window_, &attrs);
    canvas_.render(frame);
    auto pixels = canvas_.scaled(attrs.width, attrs.height);
    auto *data = static_cast<char *>(std::malloc(pixels.size() * 4));
    if (!data)
        throw std::bad_alloc();
    std::memcpy(data, pixels.data(), pixels.size() * 4);
    auto *image = XCreateImage(display_, attrs.visual, static_cast<unsigned>(attrs.depth), ZPixmap,
                               0, data, static_cast<unsigned>(attrs.width),
                               static_cast<unsigned>(attrs.height), 32, 0);
    if (!image) {
        std::free(data);
        throw std::runtime_error("Xlib image allocation failed");
    }
    // XImage owns this allocation; do not release the vector's storage through XDestroyImage.
    if (image->bits_per_pixel != 32 || image->red_mask != 0xff0000 || image->green_mask != 0xff00 ||
        image->blue_mask != 0xff) {
        XDestroyImage(image);
        throw std::runtime_error("Xlib requires a 24/32-bit TrueColor display");
    }
    XPutImage(display_, window_, gc_, image, 0, 0, 0, 0, static_cast<unsigned>(attrs.width),
              static_cast<unsigned>(attrs.height));
    XDestroyImage(image);
    XFlush(display_);
}
} // namespace arcade::bonus
ARCADE_PLUGIN(arcade::bonus::Xlib, arcade::PluginKind::Graphics, "xlib", "XLIB / NATIVE X11")
