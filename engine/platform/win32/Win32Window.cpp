#include "forge11/platform/Win32Window.h"

#define NOMINMAX
#include <windows.h>

namespace forge11::platform {

namespace {

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
}

} // namespace

struct Win32Window::Impl {
    HWND hwnd = nullptr;
};

Win32Window::Win32Window() : m_impl(std::make_unique<Impl>()) {}

Win32Window::~Win32Window() {
    if (m_impl->hwnd) {
        DestroyWindow(m_impl->hwnd);
    }
}

bool Win32Window::create(const std::wstring& title, int width, int height) {
    HINSTANCE hInstance = GetModuleHandleW(nullptr);

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"Forge11WindowClass";
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);

    RegisterClassExW(&wc);

    m_impl->hwnd = CreateWindowExW(
        0,
        wc.lpszClassName,
        title.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        width, height,
        nullptr, nullptr, hInstance, nullptr);

    if (!m_impl->hwnd) {
        return false;
    }

    ShowWindow(m_impl->hwnd, SW_SHOW);
    return true;
}

bool Win32Window::pollEvents() {
    MSG msg{};
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            return false;
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return true;
}

void* Win32Window::nativeHandle() const {
    return m_impl->hwnd;
}

} // namespace forge11::platform