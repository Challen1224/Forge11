#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace forge11::ui::widgets {

/// Base class for all Forge11 UI widgets. Holds common properties
/// (name, size, children) shared by every widget type.
class Widget {
public:
    explicit Widget(std::string typeName) : m_typeName(std::move(typeName)) {}
    virtual ~Widget() = default;

    Widget(const Widget&) = delete;
    Widget& operator=(const Widget&) = delete;

    const std::string& typeName() const { return m_typeName; }

    const std::string& name() const { return m_name; }
    void setName(std::string name) { m_name = std::move(name); }

    int32_t width() const { return m_width; }
    void setWidth(int32_t w) { m_width = w; }

    int32_t height() const { return m_height; }
    void setHeight(int32_t h) { m_height = h; }

    void addChild(std::unique_ptr<Widget> child) {
        child->m_parent = this;
        m_children.push_back(std::move(child));
    }

    const std::vector<std::unique_ptr<Widget>>& children() const { return m_children; }
    Widget* parent() const { return m_parent; }

private:
    std::string m_typeName;
    std::string m_name;
    int32_t m_width = 0;
    int32_t m_height = 0;
    Widget* m_parent = nullptr;
    std::vector<std::unique_ptr<Widget>> m_children;
};

} // namespace forge11::ui::widgets