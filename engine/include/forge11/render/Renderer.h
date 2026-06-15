#pragma once

#include "forge11/render/Color.h"
#include "forge11/ui/layout/LayoutRect.h"

#include <memory>
#include <cstdint>
#include <vector>

namespace forge11::render {

/// Minimal D3D12 renderer: device, command queue, swap chain, and
/// solid-color quad rendering for the Forge11 widget tree.
class Renderer {
public:
    Renderer();
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    /// Creates the D3D12 device, command queue, swap chain, and quad pipeline.
    bool initialize(void* hwnd, uint32_t width, uint32_t height);

    /// Resizes swap chain buffers (call on WM_SIZE).
    void resize(uint32_t width, uint32_t height);

    /// Queues a solid-color rectangle for this frame, in pixel coordinates
    /// (origin top-left, matching widget layout rects).
    void drawRect(const ui::layout::LayoutRect& rect, const Color& color);

    /// Clears the backbuffer, draws all queued rects, presents, and
    /// clears the queue for the next frame.
    void renderFrame(float clearR, float clearG, float clearB, float clearA);

    /// Waits for the GPU to finish all outstanding work (call before destruction/resize).
    void waitForGpu();

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace forge11::render