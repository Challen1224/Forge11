#include "forge11/Application.h"
#include "forge11/platform/Win32Window.h"
#include "forge11/render/Renderer.h"

namespace forge11 {

struct Application::Impl {
    platform::Win32Window window;
    render::Renderer renderer;
    bool running = false;
};

Application::Application() : m_impl(std::make_unique<Impl>()) {}
Application::~Application() = default;

bool Application::initialize(const std::wstring& title, int width, int height) {
    if (!m_impl->window.create(title, width, height)) {
        return false;
    }
    return m_impl->renderer.initialize(m_impl->window.nativeHandle(),
                                        static_cast<uint32_t>(width),
                                        static_cast<uint32_t>(height));
}

int Application::run() {
    m_impl->running = true;
    while (m_impl->running) {
        if (!m_impl->window.pollEvents()) {
            m_impl->running = false;
            break;
        }
        m_impl->renderer.renderFrame(0.1f, 0.1f, 0.15f, 1.0f);
    }
    m_impl->renderer.waitForGpu();
    return 0;
}

void Application::quit() {
    m_impl->running = false;
}

} // namespace forge11