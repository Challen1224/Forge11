#include "forge11/ui/layout/LayoutEngine.h"
#include "forge11/ui/widgets/Panel.h"
#include <algorithm>

namespace forge11::ui::layout {

void LayoutEngine::solve(const widgets::Widget& root, LayoutRect bounds) {
    m_rects.clear();
    solveNode(root, bounds);
}

LayoutRect LayoutEngine::rectFor(const widgets::Widget& widget) const {
    auto it = m_rects.find(&widget);
    return it != m_rects.end() ? it->second : LayoutRect{};
}

void LayoutEngine::solveNode(const widgets::Widget& node, LayoutRect bounds) {
    m_rects[&node] = bounds;

    const auto* panel = dynamic_cast<const widgets::Panel*>(&node);
    if (!panel) {
        // Leaf widget (or unhandled container): just record its bounds.
        // Recurse anyway in case it has children with their own layout.
        for (const auto& child : node.children()) {
            solveNode(*child, bounds);
        }
        return;
    }

    const auto& children = panel->children();
    if (children.empty()) {
        return;
    }

    if (panel->orientation() == widgets::Orientation::Vertical) {
        // Count children needing equal-share height (Height == 0)
        int32_t fixedHeightTotal = 0;
        int32_t flexCount = 0;
        for (const auto& child : children) {
            if (child->height() > 0) {
                fixedHeightTotal += child->height();
            } else {
                ++flexCount;
            }
        }

        int32_t remaining = bounds.height - fixedHeightTotal;
        int32_t flexHeight = flexCount > 0
            ? std::max(remaining / flexCount, 0)
            : 0;
        if (flexCount == 0) {
            flexHeight = kDefaultWidgetHeight;
        }

        int32_t cursorY = bounds.y;
        for (const auto& child : children) {
            int32_t h = child->height() > 0 ? child->height() : flexHeight;
            int32_t w = child->width() > 0 ? child->width() : bounds.width;

            LayoutRect childBounds{bounds.x, cursorY, w, h};
            solveNode(*child, childBounds);
            cursorY += h;
        }
    } else {
        // Horizontal: same logic, swapped axes
        int32_t fixedWidthTotal = 0;
        int32_t flexCount = 0;
        for (const auto& child : children) {
            if (child->width() > 0) {
                fixedWidthTotal += child->width();
            } else {
                ++flexCount;
            }
        }

        int32_t remaining = bounds.width - fixedWidthTotal;
        int32_t flexWidth = flexCount > 0
            ? std::max(remaining / flexCount, 0)
            : 0;
        if (flexCount == 0) {
            flexWidth = kDefaultWidgetWidth;
        }

        int32_t cursorX = bounds.x;
        for (const auto& child : children) {
            int32_t w = child->width() > 0 ? child->width() : flexWidth;
            int32_t h = child->height() > 0 ? child->height() : bounds.height;

            LayoutRect childBounds{cursorX, bounds.y, w, h};
            solveNode(*child, childBounds);
            cursorX += w;
        }
    }
}

} // namespace forge11::ui::layout