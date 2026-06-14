#include "forge11/Application.h"

int main() {
    forge11::Application app;
    if (!app.initialize(L"Forge11 - Hello Window", 1024, 600)) {
        return 1;
    }
    return app.run();
}