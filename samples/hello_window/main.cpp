#include "forge11/Application.h"
#include "forge11/ui/dsl/F11Parser.h"
#include "forge11/ui/WidgetFactory.h"
#include "forge11/ui/widgets/Window.h"
#include "forge11/ui/widgets/Panel.h"
#include "forge11/ui/widgets/Label.h"
#include "forge11/ui/widgets/Button.h"
#include "forge11/ui/layout/LayoutEngine.h"

#include <iostream>
#include <filesystem>

namespace {

void printWidgetTree(const forge11::ui::widgets::Widget& widget,
                      const forge11::ui::layout::LayoutEngine& layout,
                      int depth = 0) {
    auto rect = layout.rectFor(widget);

    std::cout << std::string(depth * 2, ' ')
              << widget.typeName()
              << " @(" << rect.x << "," << rect.y << ") "
              << rect.width << "x" << rect.height;

    if (!widget.name().empty()) {
        std::cout << " [Name=" << widget.name() << "]";
    }

    if (const auto* label = dynamic_cast<const forge11::ui::widgets::Label*>(&widget)) {
        std::cout << " Text=\"" << label->text() << "\"";
    }
    if (const auto* button = dynamic_cast<const forge11::ui::widgets::Button*>(&widget)) {
        std::cout << " Text=\"" << button->text() << "\"";
    }
    if (const auto* window = dynamic_cast<const forge11::ui::widgets::Window*>(&widget)) {
        std::cout << " Title=\"" << window->title() << "\"";
    }

    std::cout << "\n";

    for (const auto& child : widget.children()) {
        printWidgetTree(*child, layout, depth + 1);
    }
}

} // namespace

int main() {
    auto exeDir = std::filesystem::path(__FILE__).parent_path();

    forge11::ui::dsl::F11Parser parser;
    auto parseResult = parser.parseFile((exeDir / "app.f11").string());
    if (!parseResult) {
        std::cerr << "Parse error: " << parser.lastError() << "\n";
        return 1;
    }

    forge11::ui::WidgetFactory factory;
    auto widgetTree = factory.build(**parseResult);
    if (!widgetTree) {
        std::cerr << "Widget build error: " << factory.lastError() << "\n";
        return 1;
    }

    forge11::ui::layout::LayoutEngine layout;
    auto* window = dynamic_cast<forge11::ui::widgets::Window*>(widgetTree.get());
    forge11::ui::layout::LayoutRect rootBounds{0, 0, window->width(), window->height()};
    layout.solve(*widgetTree, rootBounds);

    std::cout << "Widget tree (with layout):\n";
    printWidgetTree(*widgetTree, layout);

    forge11::Application app;
    if (!app.initialize(L"Forge11 - Hello Window", 1024, 600)) {
        return 1;
    }

    app.setRootWidget(widgetTree.get(), &layout);

    return app.run();
}