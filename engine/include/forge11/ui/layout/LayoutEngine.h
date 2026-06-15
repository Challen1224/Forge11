#pragma once

#include "forge11/ui/layout/LayoutRect.h"
#include "forge11/ui/widgets/Widget.h"
#include <unordered_map>

namespace forge11::ui::layout {

/// Computes screen-space rectangles for every widget in a tree.
///
/// Phase 1 algorithm: simple stack layout.
/// - Panel (Vertical): children stacked top-to-bottom, full panel width,
///   each child's height = its specified Height, or an equal share of
///   remaining space if Height == 0.
/// - Panel (Horizontal): same, swapped to left-to-right / width.
/// - Leaf widgets: use their own Width/Height, falling back to a default
///   (120x32) if unspecified.
class LayoutEngine {
public:
    /// Default size used when a widget specifies Width/Height == 0.
    static constexpr int32_t kDefaultWidgetWidth = 120;
    static constexpr int32_t kDefaultWidgetHeight = 32;

    /// Recursively computes layout rects for 'root' and all descendants,
    /// starting from 'bounds'. Results are stored internally and retrievable
    /// via rectFor().
    void solve(const widgets::Widget& root, LayoutRect bounds);

    /// Returns the computed rect for the given widget, or a zeroed rect
    /// if it was not part of the last solve() call.
    LayoutRect rectFor(const widgets::Widget& widget) const;

private:
    void solveNode(const widgets::Widget& node, LayoutRect bounds);

    std::unordered_map<const widgets::Widget*, LayoutRect> m_rects;
};
}