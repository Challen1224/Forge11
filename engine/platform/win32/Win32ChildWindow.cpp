#include "forge11/platform/Win32ChildWindow.h"

#define NOMINMAX
#include <windows.h>

namespace forge11::platform {

namespace {

LRESULT CALLBACK ChildWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_ERASEBKGND:
            return 1; // prevent flicker — D3D12 owns the surface
        default:
            return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}

} // namespace

struct Win32ChildWindow::Impl {
    HWND hwnd = nullptr;
};

Win32ChildWindow::Win32ChildWindow() : m_impl(std::make_unique<Impl>()) {}

Win32ChildWindow::~Win32ChildWindow() {
    if (m_impl->hwnd) {
        DestroyWindow(m_impl->hwnd);
        m_impl->hwnd = nullptr;
    }
}

bool Win32ChildWindow::create(void* parentHwnd, int width, int height) {
    HINSTANCE hInstance = GetModuleHandleW(nullptr);

    WNDCLASSEXW wc{};
    wc.cbSize        = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc   = ChildWndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = L"Forge11ChildWindowClass";
    wc.hbrBackground = nullptr;

    RegisterClassExW(&wc); // ok if already registered

    m_impl->hwnd = CreateWindowExW(
        0,
        wc.lpszClassName,
        L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        0, 0, width, height,
        static_cast<HWND>(parentHwnd),
        nullptr,
        hInstance,
        nullptr);

    return m_impl->hwnd != nullptr;
}

void* Win32ChildWindow::nativeHandle() const {
    return m_impl->hwnd;
}

} // namespace forge11::platform