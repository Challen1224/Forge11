#pragma once

#include <memory>
#include <cstdint>

namespace forge11::platform {

/// A Win32 child window suitable for embedding inside a WPF HwndHost.
/// Unlike Win32Window (which creates a top-level WS_OVERLAPPEDWINDOW),
/// this creates a WS_CHILD | WS_VISIBLE window with no decorations.
class Win32ChildWindow {
public:
    Win32ChildWindow();
    ~Win32ChildWindow();

    Win32ChildWindow(const Win32ChildWindow&) = delete;
    Win32ChildWindow& operator=(const Win32ChildWindow&) = delete;

    bool create(void* parentHwnd, int width, int height);

    void* nativeHandle() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace forge11::platform