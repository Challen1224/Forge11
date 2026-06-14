#pragma once

#include "F11Node.h"
#include <string>
#include <memory>
#include <optional>

namespace forge11::ui::dsl {

/// Parses .f11 layout source (a constrained XML dialect) into an F11Node tree.
/// Phase 1: supports elements, attributes, self-closing and nested tags,
/// and double-quoted attribute values. No comments, namespaces, or CDATA yet.
class F11Parser {
public:
    /// Parses the given source string. Returns nullopt on syntax error,
    /// with the error message available via lastError().
    std::optional<std::unique_ptr<F11Node>> parse(const std::string& source);

    /// Loads and parses a .f11 file from disk.
    std::optional<std::unique_ptr<F11Node>> parseFile(const std::string& path);

    /// Returns a human-readable description of the last parse error, if any.
    const std::string& lastError() const { return m_lastError; }

private:
    std::string m_lastError;
};

} // namespace forge11::ui::dsl