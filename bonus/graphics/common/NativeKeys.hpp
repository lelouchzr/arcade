#pragma once
#include "arcade/Api.hpp"

namespace arcade::bonus {
// X11-compatible key symbols are also emitted by GTK and translated by Qt/Irrlicht.
Input nativeKey(unsigned key, bool pressed = true);
} // namespace arcade::bonus
