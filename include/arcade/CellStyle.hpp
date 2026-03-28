#pragma once

#include "Api.hpp"

namespace arcade {

struct CellStyle {
    Color fill;
    Color ink;
    char glyph = 0;
};

CellStyle cellStyle(Cell cell);

} // namespace arcade
