#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

namespace forge11::ui::dsl {

/// A single node in a parsed .f11 layout tree (e.g. <Button Text="OK"/>).
/// Mirrors XML element structure: a tag name, attributes, and children.
struct F11Node {
    std::string tag;
    std::unordered_map<std::string, std::string> attributes;
    std::vector<std::unique_ptr<F11Node>> children;

    /// Returns the attribute value, or `fallback` if not present.
    const std::string& attr(const std::string& name, const std::string& fallback = "") const {
        auto it = attributes.find(name);
        return it != attributes.end() ? it->second : fallback;
    }

    bool hasAttr(const std::string& name) const {
        return attributes.find(name) != attributes.end();
    }
};

} // namespace forge11::ui::dsl