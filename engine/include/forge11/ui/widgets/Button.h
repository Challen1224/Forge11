#pragma once

#include "forge11/ui/widgets/Widget.h"

namespace forge11::ui::widgets {

/// Clickable button widget with a text label.
class Button : public Widget {
public:
    Button() : Widget("Button") {}

    const std::string& text() const { return m_text; }
    void setText(std::string text) { m_text = std::move(text); }

private:
    std::string m_text;
};

} // namespace forge11::ui::widgets