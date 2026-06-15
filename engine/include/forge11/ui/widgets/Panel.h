#pragma once

#include "forge11/ui/widgets/Widget.h"

namespace forge11::ui::widgets {

enum class Orientation {
    Horizontal,
    Vertical
};

/// Container widget that lays out its children either horizontally or vertically.
/// Layout positioning is computed separately by the layout solver (Phase 2).
class Panel : public Widget {
public:
    Panel() : Widget("Panel") {}

    Orientation orientation() const { return m_orientation; }
    void setOrientation(Orientation o) { m_orientation = o; }

private:
    Orientation m_orientation = Orientation::Vertical;
};

} // namespace forge11::ui::widgets