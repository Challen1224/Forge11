#include "forge11/ui/WidgetFactory.h"

#include "forge11/ui/widgets/Window.h"
#include "forge11/ui/widgets/Panel.h"
#include "forge11/ui/widgets/Label.h"
#include "forge11/ui/widgets/Button.h"

#include <cstdlib>

namespace forge11::ui {

namespace {

int32_t parseIntAttr(const dsl::F11Node& node, const std::string& name, int32_t fallback) {
    if (!node.hasAttr(name)) {
        return fallback;
    }
    return static_cast<int32_t>(std::strtol(node.attr(name).c_str(), nullptr, 10));
}

} // namespace

void WidgetFactory::applyCommonAttributes(widgets::Widget& widget, const dsl::F11Node& node) {
    if (node.hasAttr("Name")) {
        widget.setName(node.attr("Name"));
    }
    widget.setWidth(parseIntAttr(node, "Width", 0));
    widget.setHeight(parseIntAttr(node, "Height", 0));
}

std::unique_ptr<widgets::Widget> WidgetFactory::createWidget(const dsl::F11Node& node) {
    if (node.tag == "Window") {
        auto window = std::make_unique<widgets::Window>();
        window->setTitle(node.attr("Title"));
        return window;
    }

    if (node.tag == "Panel") {
        auto panel = std::make_unique<widgets::Panel>();
        const std::string& orientation = node.attr("Orientation", "Vertical");
        panel->setOrientation(orientation == "Horizontal"
                                   ? widgets::Orientation::Horizontal
                                   : widgets::Orientation::Vertical);
        return panel;
    }

    if (node.tag == "Label") {
        auto label = std::make_unique<widgets::Label>();
        label->setText(node.attr("Text"));
        return label;
    }

    if (node.tag == "Button") {
        auto button = std::make_unique<widgets::Button>();
        button->setText(node.attr("Text"));
        return button;
    }

    m_lastError = "Unknown widget type: <" + node.tag + ">";
    return nullptr;
}

std::unique_ptr<widgets::Widget> WidgetFactory::build(const dsl::F11Node& node) {
    auto widget = createWidget(node);
    if (!widget) {
        return nullptr; // m_lastError already set
    }

    applyCommonAttributes(*widget, node);

    for (const auto& childNode : node.children) {
        auto childWidget = build(*childNode);
        if (!childWidget) {
            return nullptr; // propagate error
        }
        widget->addChild(std::move(childWidget));
    }

    return widget;
}

} // namespace forge11::ui