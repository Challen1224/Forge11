#pragma once

#include <memory>
#include <string>

namespace forge11::ui::widgets { class Widget; }
namespace forge11::ui::layout { class LayoutEngine; }

namespace forge11 {

/// Owns the main event loop and top-level application state.
/// One Application per process.
class Application {
public:
    Application();
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    /// Creates the primary window and D3D12 renderer. Returns false on failure.
    bool initialize(const std::wstring& title, int width, int height);

    /// Sets the widget tree to render each frame. Both pointers must
    /// outlive the Application (or be cleared before destruction).
    void setRootWidget(ui::widgets::Widget* root, ui::layout::LayoutEngine* layout);

    /// Runs the platform message loop + render loop until the application exits.
    /// Returns the process exit code.
    int run();

    /// Requests application shutdown; safe to call from anywhere.
    void quit();

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace forge11