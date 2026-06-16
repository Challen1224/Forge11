#include "forge11/abi/forge11_abi.h"
#include "forge11/Application.h"
#include "forge11/render/View.h"

namespace { constexpr const char* kVersion = "0.1.0"; }

// ------------------------------------------------------------------ //
//  Application
// ------------------------------------------------------------------ //
Forge11AppHandle forge11_app_create() {
    return new forge11::Application();
}
int forge11_app_initialize(Forge11AppHandle app, const wchar_t* title,
                            int width, int height) {
    if (!app) return 0;
    return static_cast<forge11::Application*>(app)
        ->initialize(title ? title : L"Forge11", width, height) ? 1 : 0;
}
int forge11_app_run(Forge11AppHandle app) {
    if (!app) return -1;
    return static_cast<forge11::Application*>(app)->run();
}
void forge11_app_quit(Forge11AppHandle app) {
    if (app) static_cast<forge11::Application*>(app)->quit();
}
void forge11_app_destroy(Forge11AppHandle app) {
    delete static_cast<forge11::Application*>(app);
}
const char* forge11_get_version() { return kVersion; }

// ------------------------------------------------------------------ //
//  Embedded view
// ------------------------------------------------------------------ //
Forge11ViewHandle forge11_view_create(void* parentHwnd, int width, int height) {
    auto* view = new forge11::render::View();
    if (!view->initialize(parentHwnd, width, height)) {
        delete view;
        return nullptr;
    }
    return view;
}
void forge11_view_tick(Forge11ViewHandle view, float r, float g, float b) {
    if (view) static_cast<forge11::render::View*>(view)->tick(r, g, b);
}
void forge11_view_resize(Forge11ViewHandle view, int width, int height) {
    if (view) static_cast<forge11::render::View*>(view)->resize(width, height);
}
void* forge11_view_hwnd(Forge11ViewHandle view) {
    if (!view) return nullptr;
    return static_cast<forge11::render::View*>(view)->hwnd();
}
void forge11_view_destroy(Forge11ViewHandle view) {
    if (view) {
        static_cast<forge11::render::View*>(view)->waitForGpu();
        delete static_cast<forge11::render::View*>(view);
    }
}