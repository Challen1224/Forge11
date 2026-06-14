#pragma once

#include <memory>
#include <string>

namespace forge11::platform {

/// Thin RAII wrapper around a Win32 HWND top-level window.
/// Phase 1: creates a blank window and pumps messages.
class Win32Window {
public:
    Win32Window();
    ~Win32Window();

    Win32Window(const Win32Window&) = delete;
    Win32Window& operator=(const Win32Window&) = delete;

    bool create(const std::wstring& title, int width, int height);

    /// Processes all pending messages. Returns false when WM_QUIT received.
    bool pollEvents();

    /// Native window handle, exposed as void* to avoid leaking <windows.h>.
    void* nativeHandle() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace forge11::platform