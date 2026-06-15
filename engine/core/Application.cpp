#include "forge11/Application.h"
#include "forge11/platform/Win32Window.h"
#include "forge11/render/Renderer.h"
#include "forge11/ui/widgets/Widget.h"
#include "forge11/ui/widgets/Panel.h"
#include "forge11/ui/widgets/Label.h"
#include "forge11/ui/widgets/Button.h"
#include "forge11/ui/layout/LayoutEngine.h"

namespace forge11 {

struct Application::Impl {
    platform::Win32Window window;
    render::Renderer renderer;
    bool running = false;

    // Set externally before run() to render a widget tree each frame.
    ui::widgets::Widget* rootWidget = nullptr;
    ui::layout::LayoutEngine* layoutEngine = nullptr;
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

void Application::setRootWidget(ui::widgets::Widget* root, ui::layout::LayoutEngine* layout) {
    m_impl->rootWidget = root;
    m_impl->layoutEngine = layout;
}

namespace {

render::Color colorForWidget(const ui::widgets::Widget& widget) {
    if (dynamic_cast<const ui::widgets::Panel*>(&widget)) {
        return {0.15f, 0.15f, 0.18f, 1.0f}; // dark gray
    }
    if (dynamic_cast<const ui::widgets::Label*>(&widget)) {
        return {0.2f, 0.4f, 0.7f, 1.0f}; // blue
    }
    if (dynamic_cast<const ui::widgets::Button*>(&widget)) {
        return {0.3f, 0.7f, 0.3f, 1.0f}; // green
    }
    return {0.4f, 0.4f, 0.4f, 1.0f}; // default gray
}

void drawWidgetTree(render::Renderer& renderer,
                     const ui::widgets::Widget& widget,
                     const ui::layout::LayoutEngine& layout) {
    // Skip the root Window itself (no visible rect of its own).
    if (widget.parent() != nullptr) {
        renderer.drawRect(layout.rectFor(widget), colorForWidget(widget));
    }
    for (const auto& child : widget.children()) {
        drawWidgetTree(renderer, *child, layout);
    }
}

} // namespace

int Application::run() {
    m_impl->running = true;
    while (m_impl->running) {
        if (!m_impl->window.pollEvents()) {
            m_impl->running = false;
            break;
        }

        if (m_impl->rootWidget && m_impl->layoutEngine) {
            drawWidgetTree(m_impl->renderer, *m_impl->rootWidget, *m_impl->layoutEngine);
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