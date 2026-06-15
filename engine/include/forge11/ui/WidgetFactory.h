#pragma once

#include "forge11/ui/dsl/F11Node.h"
#include "forge11/ui/widgets/Widget.h"
#include <memory>
#include <string>

namespace forge11::ui {

/// Builds a live Widget tree from a parsed .f11 F11Node tree.
/// Unknown tag names produce an error (returns nullptr, sets lastError()).
class WidgetFactory {
public:
    /// Recursively builds a Widget tree from the given root node.
    /// Returns nullptr on failure; check lastError() for details.
    std::unique_ptr<widgets::Widget> build(const dsl::F11Node& node);

    const std::string& lastError() const { return m_lastError; }

private:
    std::unique_ptr<widgets::Widget> createWidget(const dsl::F11Node& node);
    void applyCommonAttributes(widgets::Widget& widget, const dsl::F11Node& node);

    std::string m_lastError;
};

} // namespace forge11::ui