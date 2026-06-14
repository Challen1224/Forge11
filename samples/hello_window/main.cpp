#include "forge11/Application.h"
#include "forge11/ui/dsl/F11Parser.h"

#include <iostream>
#include <functional>

namespace {

void printTree(const forge11::ui::dsl::F11Node& node, int depth = 0) {
    std::cout << std::string(depth * 2, ' ') << "<" << node.tag;
    for (const auto& [name, value] : node.attributes) {
        std::cout << " " << name << "=\"" << value << "\"";
    }
    std::cout << ">\n";
    for (const auto& child : node.children) {
        printTree(*child, depth + 1);
    }
}

} // namespace

int main() {
    forge11::ui::dsl::F11Parser parser;
    auto result = parser.parseFile("app.f11");
    if (!result) {
        std::cerr << "Parse error: " << parser.lastError() << "\n";
        return 1;
    }

    std::cout << "Parsed .f11 tree:\n";
    printTree(**result);

    forge11::Application app;
    if (!app.initialize(L"Forge11 - Hello Window", 1024, 600)) {
        return 1;
    }
    return app.run();
}