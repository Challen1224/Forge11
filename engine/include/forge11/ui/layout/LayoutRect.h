#pragma once

#include <cstdint>

namespace forge11::ui::layout {

/// Axis-aligned rectangle describing a widget's computed screen position and size.
struct LayoutRect {
    int32_t x = 0;
    int32_t y = 0;
    int32_t width = 0;
    int32_t height = 0;
};

} // namespace forge11::ui::layout