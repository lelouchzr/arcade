#include "Digest.hpp"
#include <bit>

namespace arcade::benchmark {

void Digest::add(std::uint64_t value) {
    for (int byte = 0; byte < 8; ++byte) {
        value_ ^= (value >> (byte * 8)) & 255;
        value_ *= 1099511628211ULL;
    }
}

void Digest::add(std::string_view value) {
    add(value.size());
    for (char character : value) {
        value_ ^= static_cast<unsigned char>(character);
        value_ *= 1099511628211ULL;
    }
}

void Digest::add(const Snapshot &s) {
    add(static_cast<std::uint64_t>(s.width));
    add(static_cast<std::uint64_t>(s.height));
    add(static_cast<std::uint64_t>(s.cursor.x));
    add(static_cast<std::uint64_t>(s.cursor.y));
    add(static_cast<std::uint64_t>(s.score));
    add(s.generation);
    add(std::bit_cast<std::uint64_t>(s.secondsLeft));
    add(static_cast<std::uint64_t>(s.status));
    add(s.cells.size());
    for (auto cell : s.cells)
        add(static_cast<std::uint64_t>(cell));
    add(s.information.size());
    for (const auto &text : s.information)
        add(text);
    add(s.camera ? 1ULL : 0ULL);
    if (s.camera) {
        add(static_cast<std::uint64_t>(s.camera->center.x));
        add(static_cast<std::uint64_t>(s.camera->center.y));
        add(static_cast<std::uint64_t>(s.camera->columns));
        add(static_cast<std::uint64_t>(s.camera->rows));
        add(s.camera->isometric ? 1ULL : 0ULL);
    }
    add(s.sprites.size());
    for (const auto &sprite : s.sprites) {
        add(static_cast<std::uint64_t>(sprite.position.x));
        add(static_cast<std::uint64_t>(sprite.position.y));
        add(static_cast<std::uint64_t>(sprite.cell));
        add(std::bit_cast<std::uint64_t>(sprite.elevation));
    }
}

void Digest::add(const Frame &f) {
    add(f.player);
    add(f.title);
    add(f.quads.size());
    for (const auto &q : f.quads) {
        add(static_cast<std::uint64_t>(q.x));
        add(static_cast<std::uint64_t>(q.y));
        add(static_cast<std::uint64_t>(q.width));
        add(static_cast<std::uint64_t>(q.height));
        add(q.color.r);
        add(q.color.g);
        add(q.color.b);
        add(q.outline ? 1ULL : 0ULL);
    }
    add(f.labels.size());
    for (const auto &label : f.labels) {
        add(static_cast<std::uint64_t>(label.x));
        add(static_cast<std::uint64_t>(label.y));
        add(static_cast<std::uint64_t>(label.scale));
        add(label.text);
        add(label.color.r);
        add(label.color.g);
        add(label.color.b);
    }
    add(f.board ? 1ULL : 0ULL);
    if (f.board)
        add(*f.board);
}

std::uint64_t Digest::value() const {
    return value_;
}

} // namespace arcade::benchmark
