#pragma once

#include "forge11/ui/widgets/Widget.h"

namespace forge11::ui::widgets {

/// Static text display widget.
class Label : public Widget {
public:
    Label() : Widget("Label") {}

    const std::string& text() const { return m_text; }
    void setText(std::string text) { m_text = std::move(text); }

private:
    std::string m_text;
};

} // namespace forge11::ui::widgets