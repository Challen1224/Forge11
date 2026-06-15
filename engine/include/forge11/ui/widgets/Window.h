#pragma once

#include "forge11/ui/widgets/Widget.h"

namespace forge11::ui::widgets {

/// Top-level window widget. Root of every .f11 layout tree.
class Window : public Widget {
public:
    Window() : Widget("Window") {}

    const std::string& title() const { return m_title; }
    void setTitle(std::string title) { m_title = std::move(title); }

private:
    std::string m_title;
};

} // namespace forge11::ui::widgets