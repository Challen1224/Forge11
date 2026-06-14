#include "forge11/Application.h"
#include "forge11/platform/Win32Window.h"

namespace forge11 {

struct Application::Impl {
    platform::Win32Window window;
    bool running = false;
};

Application::Application() : m_impl(std::make_unique<Impl>()) {}
Application::~Application() = default;

bool Application::initialize(const std::wstring& title, int width, int height) {
    return m_impl->window.create(title, width, height);
}

int Application::run() {
    m_impl->running = true;
    while (m_impl->running) {
        if (!m_impl->window.pollEvents()) {
            m_impl->running = false;
        }
    }
    return 0;
}

void Application::quit() {
    m_impl->running = false;
}

} // namespace forge11