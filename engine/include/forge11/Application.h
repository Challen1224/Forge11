#pragma once

#include <memory>
#include <string>

namespace forge11 {
class Window;

/// Owns the main event loop and top-level application state.
/// One application per process - thats how its done for now
class Application {
public:
    Application();
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    /// Creates the primary window. Returns false on failure
    bool initialize(const std::wstring& title, int width, int height);

    /// Runs the platform message loop until the application exits
    /// Returns the process exit code
    int run();

    /// Requests application shutdown; safe to call from anywhere :)
    void quit();

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace forge11