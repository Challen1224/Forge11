#include "forge11/ui/dsl/F11Parser.h"

#include <functional>
#include <fstream>
#include <sstream>
#include <cctype>

namespace forge11::ui::dsl {

namespace {

void skipWhitespace(const std::string& src, size_t& pos) {
    while (pos < src.size() && std::isspace(static_cast<unsigned char>(src[pos]))) {
        ++pos;
    }
}

bool isNameChar(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-' || c == '.' || c == ':';
}

std::string parseName(const std::string& src, size_t& pos) {
    size_t start = pos;
    while (pos < src.size() && isNameChar(src[pos])) {
        ++pos;
    }
    return src.substr(start, pos - start);
}

} // namespace

std::optional<std::unique_ptr<F11Node>> F11Parser::parse(const std::string& source) {
    size_t pos = 0;
    skipWhitespace(source, pos);

    if (pos >= source.size() || source[pos] != '<') {
        m_lastError = "Expected '<' at start of document";
        return std::nullopt;
    }

    // Recursive descent: parseElement returns nullptr on error and sets m_lastError.
    std::function<std::unique_ptr<F11Node>(size_t&)> parseElement =
        [&](size_t& p) -> std::unique_ptr<F11Node> {

        skipWhitespace(source, p);
        if (p >= source.size() || source[p] != '<') {
            m_lastError = "Expected '<' to start element";
            return nullptr;
        }
        ++p; // consume '<'

        std::string tag = parseName(source, p);
        if (tag.empty()) {
            m_lastError = "Expected element name after '<'";
            return nullptr;
        }

        auto node = std::make_unique<F11Node>();
        node->tag = tag;

        // Parse attributes
        for (;;) {
            skipWhitespace(source, p);
            if (p >= source.size()) {
                m_lastError = "Unexpected end of input while parsing attributes of <" + tag + ">";
                return nullptr;
            }

            if (source[p] == '/' || source[p] == '>') {
                break;
            }

            std::string attrName = parseName(source, p);
            if (attrName.empty()) {
                m_lastError = "Expected attribute name in <" + tag + ">";
                return nullptr;
            }

            skipWhitespace(source, p);
            if (p >= source.size() || source[p] != '=') {
                m_lastError = "Expected '=' after attribute '" + attrName + "' in <" + tag + ">";
                return nullptr;
            }
            ++p; // consume '='

            skipWhitespace(source, p);
            if (p >= source.size() || source[p] != '"') {
                m_lastError = "Expected '\"' to start value of attribute '" + attrName + "' in <" + tag + ">";
                return nullptr;
            }
            ++p; // consume opening quote

            size_t valueStart = p;
            while (p < source.size() && source[p] != '"') {
                ++p;
            }
            if (p >= source.size()) {
                m_lastError = "Unterminated attribute value for '" + attrName + "' in <" + tag + ">";
                return nullptr;
            }
            std::string attrValue = source.substr(valueStart, p - valueStart);
            ++p; // consume closing quote

            node->attributes.emplace(std::move(attrName), std::move(attrValue));
        }

        // Self-closing tag: <Tag .../>
        if (source[p] == '/') {
            ++p;
            if (p >= source.size() || source[p] != '>') {
                m_lastError = "Expected '>' after '/' in <" + tag + ">";
                return nullptr;
            }
            ++p; // consume '>'
            return node;
        }

        // Opening tag closed: <Tag ...>
        ++p; // consume '>'

        // Parse children and text until closing tag </Tag>
        for (;;) {
            skipWhitespace(source, p);
            if (p >= source.size()) {
                m_lastError = "Unexpected end of input; missing closing tag for <" + tag + ">";
                return nullptr;
            }

            if (source[p] == '<') {
                if (p + 1 < source.size() && source[p + 1] == '/') {
                    // Closing tag: </Tag>
                    size_t closeStart = p + 2;
                    size_t closeEnd = closeStart;
                    std::string closeName = parseName(source, closeEnd);
                    if (closeName != tag) {
                        m_lastError = "Mismatched closing tag: expected </" + tag + ">, got </" + closeName + ">";
                        return nullptr;
                    }
                    p = closeEnd;
                    skipWhitespace(source, p);
                    if (p >= source.size() || source[p] != '>') {
                        m_lastError = "Expected '>' after closing tag </" + tag + ">";
                        return nullptr;
                    }
                    ++p; // consume '>'
                    return node;
                }

                auto child = parseElement(p);
                if (!child) {
                    return nullptr; // error already set
                }
                node->children.push_back(std::move(child));
            } else {
                // Skip text content (Phase 1: not stored as nodes)
                ++p;
            }
        }
    };

    auto root = parseElement(pos);
    if (!root) {
        return std::nullopt;
    }
    return root;
}

std::optional<std::unique_ptr<F11Node>> F11Parser::parseFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        m_lastError = "Could not open file: " + path;
        return std::nullopt;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    return parse(ss.str());
}

} // namespace forge11::ui::dsl