#pragma once

#include <memory>
#include <cstdint>

namespace forge11::render {

/// Self-contained D3D12 renderer bound to a child HWND.
/// Driven externally (no internal message loop).
/// Phase A: clear-color only.
class View {
public:
    View();
    ~View();

    View(const View&) = delete;
    View& operator=(const View&) = delete;

    bool initialize(void* parentHwnd, int width, int height);

    void tick(float r, float g, float b);
    void resize(int width, int height);
    void waitForGpu();

    void* hwnd() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace forge11::render