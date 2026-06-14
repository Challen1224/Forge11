#include "forge11/abi/forge11_abi.h"
#include "forge11/Application.h"

namespace {
constexpr const char* kVersion = "0.1.0";
}

Forge11AppHandle forge11_app_create() {
    return new forge11::Application();
}

int forge11_app_initialize(Forge11AppHandle app, const wchar_t* title, int width, int height) {
    if (!app) return 0;
    auto* instance = static_cast<forge11::Application*>(app);
    return instance->initialize(title ? title : L"Forge11", width, height) ? 1 : 0;
}

int forge11_app_run(Forge11AppHandle app) {
    if (!app) return -1;
    auto* instance = static_cast<forge11::Application*>(app);
    return instance->run();
}

void forge11_app_quit(Forge11AppHandle app) {
    if (!app) return;
    static_cast<forge11::Application*>(app)->quit();
}

void forge11_app_destroy(Forge11AppHandle app) {
    delete static_cast<forge11::Application*>(app);
}

const char* forge11_get_version() {
    return kVersion;
}