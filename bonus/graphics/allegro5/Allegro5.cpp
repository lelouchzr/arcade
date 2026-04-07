#include "Allegro5.hpp"
#include "../common/NativeKeys.hpp"
#include "arcade/Font.hpp"
#include <algorithm>
#include <allegro5/allegro_primitives.h>
#include <stdexcept>

namespace arcade::bonus {
Allegro5::~Allegro5() {
    close();
}

void Allegro5::open() {
    if (display_)
        return;
    try {
        if (!al_init())
            throw std::runtime_error("Allegro5 initialization failed");
        initialized_ = true;
        if (!al_install_keyboard() || !al_init_primitives_addon())
            throw std::runtime_error("Allegro5 keyboard/primitives initialization failed");
        al_set_new_display_flags(ALLEGRO_RESIZABLE);
        display_ = al_create_display(Frame::Width, Frame::Height);
        if (!display_)
            throw std::runtime_error("Allegro5 display creation failed");
        al_set_window_title(display_, "ARCADE / 400 - Allegro5");
        queue_ = al_create_event_queue();
        if (!queue_)
            throw std::runtime_error("Allegro5 event queue allocation failed");
        al_register_event_source(queue_, al_get_display_event_source(display_));
        al_register_event_source(queue_, al_get_keyboard_event_source());
    } catch (...) {
        close();
        throw;
    }
}

void Allegro5::close() noexcept {
    if (queue_)
        al_destroy_event_queue(queue_);
    if (display_)
        al_destroy_display(display_);
    queue_ = nullptr;
    display_ = nullptr;
    if (initialized_) {
        al_shutdown_primitives_addon();
        al_uninstall_system();
    }
    initialized_ = false;
}

std::vector<Input> Allegro5::poll() {
    std::vector<Input> result;
    ALLEGRO_EVENT event;
    while (queue_ && al_get_next_event(queue_, &event)) {
        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            result.push_back({Action::Quit});
        if (event.type == ALLEGRO_EVENT_DISPLAY_RESIZE)
            al_acknowledge_resize(display_);
        if (event.type == ALLEGRO_EVENT_KEY_CHAR && event.keyboard.unichar > 32 &&
            event.keyboard.unichar < 127)
            result.push_back({Action::Text, static_cast<char>(event.keyboard.unichar)});
        if (event.type != ALLEGRO_EVENT_KEY_DOWN && event.type != ALLEGRO_EVENT_KEY_UP)
            continue;
        unsigned key = 0;
        switch (event.keyboard.keycode) {
        case ALLEGRO_KEY_LEFT:
            key = 0xff51;
            break;
        case ALLEGRO_KEY_UP:
            key = 0xff52;
            break;
        case ALLEGRO_KEY_RIGHT:
            key = 0xff53;
            break;
        case ALLEGRO_KEY_DOWN:
            key = 0xff54;
            break;
        case ALLEGRO_KEY_ESCAPE:
            key = 0xff1b;
            break;
        case ALLEGRO_KEY_ENTER:
            key = 0xff0d;
            break;
        case ALLEGRO_KEY_TAB:
            key = 0xff09;
            break;
        case ALLEGRO_KEY_BACKSPACE:
            key = 0xff08;
            break;
        case ALLEGRO_KEY_SPACE:
            key = ' ';
            break;
        default:
            if (event.keyboard.keycode >= ALLEGRO_KEY_F2 &&
                event.keyboard.keycode <= ALLEGRO_KEY_F7)
                key = 0xffbfU + static_cast<unsigned>(event.keyboard.keycode - ALLEGRO_KEY_F2);
        }
        auto input = nativeKey(key, event.type == ALLEGRO_EVENT_KEY_DOWN);
        if (input.action != Action::None)
            result.push_back(input);
    }
    return result;
}

void Allegro5::draw(const Frame &frame) {
    if (!display_)
        throw std::runtime_error("Allegro5 display is closed");
    const float width = static_cast<float>(al_get_display_width(display_)),
                height = static_cast<float>(al_get_display_height(display_));
    const float scale = std::min(width / Frame::Width, height / Frame::Height);
    ALLEGRO_TRANSFORM transform;
    al_identity_transform(&transform);
    al_use_transform(&transform);
    al_clear_to_color(al_map_rgb(10, 15, 23));
    al_scale_transform(&transform, scale, scale);
    al_translate_transform(&transform, (width - Frame::Width * scale) / 2,
                           (height - Frame::Height * scale) / 2);
    al_use_transform(&transform);
    auto quad = [](const Quad &q) {
        auto c = al_map_rgb(q.color.r, q.color.g, q.color.b);
        float x = static_cast<float>(q.x), y = static_cast<float>(q.y),
              r = static_cast<float>(q.x + q.width), b = static_cast<float>(q.y + q.height);
        if (q.outline)
            al_draw_rectangle(x + .5f, y + .5f, r - .5f, b - .5f, c, 1);
        else
            al_draw_filled_rectangle(x, y, r, b, c);
    };
    for (const auto &q : frame.quads)
        quad(q);
    for (const auto &label : frame.labels)
        drawLabel(label, quad);
    al_flip_display();
}
} // namespace arcade::bonus
ARCADE_PLUGIN(arcade::bonus::Allegro5, arcade::PluginKind::Graphics, "allegro5",
              "ALLEGRO5 / NATIVE PRIMITIVES")
