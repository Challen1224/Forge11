#pragma once

#include <memory>
#include <cstdint>

namespace forge11::render {

/// Minimal D3D12 renderer: device, command queue, swap chain, and a
/// per-frame clear. UI/geometry rendering builds on top of this in later phases.
class Renderer {
public:
    Renderer();
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    /// Creates the D3D12 device, command queue, and swap chain for the given HWND.
    bool initialize(void* hwnd, uint32_t width, uint32_t height);

    /// Resizes swap chain buffers (call on WM_SIZE).
    void resize(uint32_t width, uint32_t height);

    /// Clears the current backbuffer to the given color and presents.
    void renderFrame(float r, float g, float b, float a);

    /// Waits for the GPU to finish all outstanding work (call before destruction/resize).
    void waitForGpu();

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace forge11::render