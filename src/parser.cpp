#include "parser.hpp"
#include <sstream>
#include <iostream>
#include <algorithm>
#include <nlohmann/json.hpp>

namespace solum {

// Helper to wrap nodes in the correct style based on marker
static Node wrapWithMarker(const std::string& marker, const std::vector<Node>& nodes) {
    if (marker == "!!") return BoldNode{nodes};
    if (marker == "::") return ItalicNode{nodes};
    if (marker == "==") return UnderlineNode{nodes};
    if (marker == "%%") return HighlightNode{nodes};
    if (marker == "~~") return StrikeNode{nodes};
    if (marker == ",,") return SubscriptNode{nodes};
    if (marker == "^^") return SuperscriptNode{nodes};
    return TextNode{""}; // Should not happen
}

Parser::Parser() {
    headingCounts.fill(0);
}

std::string Parser::trim(const std::string& s) {
    size_t first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, (last - first + 1));
}

std::vector<Node> Parser::parseInline(const std::string& text) {
    struct Level {
        std::string marker;
        std::vector<Node> nodes;
    };
    std::vector<Level> stack;
    stack.push_back({"", {}}); // Base level
    const size_t MAX_NESTING_DEPTH = 100;

    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\\') {
            if (i + 1 < text.size()) {
                stack.back().nodes.push_back(TextNode{std::string(1, text[i + 1])});
                i++;
            } else {
                stack.back().nodes.push_back(TextNode{"\\"});
            }
            continue;
        }

        // Block markers appearing inline should be treated as literal text to avoid collisions
        if (text[i] == '=' && i + 2 < text.size() && text[i+1] == '=' && text[i+2] == '=') {
            stack.back().nodes.push_back(TextNode{"==="});
            i += 2;
            continue;
        }
        if (text[i] == '$' && i + 2 < text.size() && text[i+1] == '$' && text[i+2] == '$') {
            stack.back().nodes.push_back(TextNode{"$$$"});
            i += 2;
            continue;
        }
        if (text[i] == '{' && i + 1 < text.size() && text[i+1] == '{') {
            stack.back().nodes.push_back(TextNode{"{{"});
            i += 1;
            continue;
        }
        if (text[i] == '}' && i + 1 < text.size() && text[i+1] == '}') {
            stack.back().nodes.push_back(TextNode{"}}"});
            i += 1;
            continue;
        }

        if (text[i] == '\'' && i + 1 < text.size() && text[i + 1] == '\'') {
            i += 2;
            size_t start = i;
            while (i + 1 < text.size() && !(text[i] == '\'' && text[i + 1] == '\'')) {
                i++;
            }
            if (i + 1 < text.size()) {
                stack.back().nodes.push_back(LiteralNode{text.substr(start, i - start)});
                i++; 
            } else {
                stack.back().nodes.push_back(TextNode{text.substr(start)});
            }
            continue;
        }

        if (text[i] == '[' && i + 1 < text.size()) {
            if (text[i + 1] == '@' || text[i + 1] == '!') {
                bool isLink = (text[i + 1] == '@');
                size_t bracketEnd = text.find(']', i);
                if (bracketEnd != std::string::npos) {
                    std::string content = text.substr(i + 2, bracketEnd - (i + 2));
                    size_t firstQuote = content.find('\"');
                    size_t lastQuote = content.rfind('\"');
                    if (firstQuote != std::string::npos && lastQuote != std::string::npos && firstQuote != lastQuote) {
                        std::string path = trim(content.substr(0, firstQuote));
                        std::string label = content.substr(firstQuote + 1, lastQuote - firstQuote - 1);
                        if (isLink) stack.back().nodes.push_back(LinkNode{path, label});
                        else stack.back().nodes.push_back(ImageNode{path, label});
                        i = bracketEnd;
                        continue;
                    }
                }
            } else if (text[i + 1] == ' ' && i + 2 < text.size() && text[i + 2] == ']') {
                stack.back().nodes.push_back(TaskNode{false});
                i += 2;
                continue;
            } else if (text[i + 1] == 'x' && i + 2 < text.size() && text[i + 2] == ']') {
                stack.back().nodes.push_back(TaskNode{true});
                i += 2;
                continue;
            }
        }

        bool matched = false;
        auto handleFormat = [&](const std::string& sym, const std::string& marker, auto nodeCreator) {
            if (text[i] == sym[0] && i + 1 < text.size() && text[i + 1] == sym[1]) {
                int matchIdx = -1;
                for (int j = (int)stack.size() - 1; j >= 0; --j) {
                    if (stack[j].marker == marker) {
                        matchIdx = j;
                        break;
                    }
                }

                if (matchIdx != -1 && matchIdx > 0) {
                    std::vector<std::string> reopenedMarkers;
                    
                    while ((int)stack.size() - 1 > matchIdx) {
                        std::string currentMarker = stack.back().marker;
                        auto nodes = std::move(stack.back().nodes);
                        stack.pop_back();
                        
                        // Wrap the collapsed content in its original style node before moving it down
                        Node wrappedNode;
                        if (currentMarker == "!!") wrappedNode = nodeCreator(nodes); // This is a bit hacky because nodeCreator is for the current sym
                        // Actually, we need a way to wrap based on the marker.
                        // Let's use a helper.
                        
                        stack.back().nodes.push_back(wrapWithMarker(currentMarker, nodes));
                        reopenedMarkers.push_back(currentMarker);
                    }
                    
                    auto finalNodes = std::move(stack.back().nodes);
                    stack.pop_back();
                    stack.back().nodes.push_back(nodeCreator(finalNodes));
                    
                    for (auto it = reopenedMarkers.rbegin(); it != reopenedMarkers.rend(); ++it) {
                        stack.push_back({*it, {}});
                    }
                    
                    i++;
                    return true;
                } else if (stack.size() < MAX_NESTING_DEPTH) {
                    stack.push_back({marker, {}});
                    i++;
                    return true;
                }
            }
            return false;
        };

        if (handleFormat("!!", "!!", [](auto& n) { return BoldNode{n}; })) matched = true;
        else if (handleFormat("::", "::", [](auto& n) { return ItalicNode{n}; })) matched = true;
        else if (handleFormat("==", "==", [](auto& n) { return UnderlineNode{n}; })) matched = true;
        else if (handleFormat("%%", "%%", [](auto& n) { return HighlightNode{n}; })) matched = true;
        else if (handleFormat("~~", "~~", [](auto& n) { return StrikeNode{n}; })) matched = true;
        else if (handleFormat(",,", ",,", [](auto& n) { return SubscriptNode{n}; })) matched = true;
        else if (handleFormat("^^", "^^", [](auto& n) { return SuperscriptNode{n}; })) matched = true;
        else if (text[i] == '{') {
            i++;
            std::string content;
            bool closed = false;
            while (i < text.size()) {
                if (text[i] == '}') {
                    closed = true;
                    break;
                }
                content += text[i++];
            }
            if (closed) {
                stack.back().nodes.push_back(InlineCodeNode{content});
                matched = true;
            } else {
                stack.back().nodes.push_back(TextNode{"{" + content});
                i--; 
                matched = true;
            }
        } else if (text[i] == '$' && i + 1 < text.size() && text[i + 1] == '$') {
            size_t start = i + 2;
            size_t end = text.find("$$", start);
            if (end != std::string::npos) {
                stack.back().nodes.push_back(MathInlineNode{text.substr(start, end - start)});
                i = end + 1;
                matched = true;
            }
        }

        if (!matched) {
            if (!stack.back().nodes.empty() && std::holds_alternative<TextNode>(stack.back().nodes.back())) {
                std::get<TextNode>(stack.back().nodes.back()).content += text[i];
            } else {
                stack.back().nodes.push_back(TextNode{std::string(1, text[i])});
            }
        }
    }

    while (stack.size() > 1) {
        auto level = std::move(stack.back());
        stack.pop_back();
        if (!level.marker.empty()) {
            stack.back().nodes.push_back(TextNode{level.marker});
        }
        for (auto& n : level.nodes) {
            stack.back().nodes.push_back(std::move(n));
        }
    }

    return stack[0].nodes;
}

Document Parser::parse(const std::string& input) {
    Document doc;
    std::vector<std::string> lines;
    std::stringstream ss(input);
    std::string line;
    while (std::getline(ss, line)) lines.push_back(line);

    size_t i = 0;
    if (!lines.empty() && trim(lines[0]) == "===META") {
        i = 1;
        while (i < lines.size() && trim(lines[i]) != "===") {
            std::string metaLine = lines[i];
            size_t sep = metaLine.find(':');
            if (sep != std::string::npos) {
                doc.metadata.push_back({trim(metaLine.substr(0, sep)), trim(metaLine.substr(sep + 1))});
            }
            i++;
        }
        if (i < lines.size()) i++; // skip closing ===
    }

    std::vector<ListNode*> listStack;

    for (; i < lines.size(); ++i) {
        std::string currentLine = lines[i];
        std::string trimmed = trim(currentLine);
        
        if (trimmed.empty()) {
            doc.nodes.push_back(BlankLineNode{});
            listStack.clear();
            continue;
        }

        if (trimmed == "===") {
            std::string literalContent;
            i++;
            while (i < lines.size()) {
                if (trim(lines[i]) == "===") break;
                literalContent += lines[i] + "\n";
                i++;
            }
            doc.nodes.push_back(LiteralNode{literalContent});
            listStack.clear();
            continue;
        }

        if (trimmed.find("$$$") == 0) {
            std::string content;
            i++;
            while (i < lines.size()) {
                if (trim(lines[i]).find("$$$") == 0) break;
                content += lines[i] + "\n";
                i++;
            }
            doc.nodes.push_back(MathBlockNode{content});
            listStack.clear();
            continue;
        }

        if (trimmed.find("{{") == 0) {
            std::string lang = "text";
            size_t langEnd = trimmed.find(":", 2);
            if (langEnd != std::string::npos) lang = trimmed.substr(2, langEnd - 2);
            std::string content;
            i++;
            while (i < lines.size()) {
                if (trim(lines[i]).find("}}") == 0) break;
                content += lines[i] + "\n";
                i++;
            }
            doc.nodes.push_back(CodeBlockNode{lang, content});
            listStack.clear();
            continue;
        }

        if (trimmed == "[q]") {
            size_t startLine = i + 1;
            int depth = 1;
            size_t endLine = lines.size();
            for (size_t j = i + 1; j < lines.size(); ++j) {
                if (trim(lines[j]) == "[q]") {
                    depth--;
                    if (depth == 0) {
                        endLine = j;
                        break;
                    }
                }
            }
            
            std::string content;
            for (size_t j = startLine; j < endLine; ++j) {
                content += lines[j] + "\n";
            }
            
            Document innerDoc = parse(content);
            for (auto& node : innerDoc.nodes) {
                if (std::holds_alternative<QuoteNode>(node)) {
                    std::get<QuoteNode>(node).level += 1;
                }
            }
            
            doc.nodes.push_back(QuoteNode{1, innerDoc.nodes});
            i = endLine;
            listStack.clear();
            continue;
        }

        if (trimmed == "[t]") {
            TableNode table;
            table.hasHeader = false;
            std::vector<std::vector<std::vector<Node>>> rows;
            i++;
            while (i < lines.size()) {
                if (trim(lines[i]) == "[t]") break;
                std::string rowLine = lines[i];
                if (trim(rowLine) == "---") {
                    table.hasHeader = true;
                    i++; continue;
                }
                std::vector<std::vector<Node>> rowCells;
                size_t start = 0, pos = 0;
                int depth = 0;
                while (pos < rowLine.size()) {
                    if (rowLine[pos] == '[') depth++;
                    else if (rowLine[pos] == ']') depth--;
                    if (depth == 0 && pos + 1 < rowLine.size() && rowLine[pos] == '|' && rowLine[pos+1] == '|') {
                        rowCells.push_back(parseInline(trim(rowLine.substr(start, pos - start))));
                        start = pos + 2;
                        pos += 2;
                    } else pos++;
                }
                rowCells.push_back(parseInline(trim(rowLine.substr(start))));
                rows.push_back(rowCells);
                i++;
            }
            table.rows = rows;
            doc.nodes.push_back(table);
            listStack.clear();
            continue;
        }

        std::vector<Node> lineNodes = parseLine(currentLine, i);
        if (lineNodes.empty()) continue;

        if (std::holds_alternative<ListNode>(lineNodes[0])) {
            ListNode listNode = std::get<ListNode>(lineNodes[0]);
            int level = listNode.level;

            while (!listStack.empty() && listStack.back()->level >= level) {
                listStack.pop_back();
            }

            if (listStack.empty()) {
                doc.nodes.push_back(listNode);
                listStack.push_back(&std::get<ListNode>(doc.nodes.back()));
            } else {
                ListNode* parent = listStack.back();
                parent->nested.push_back(listNode);
                listStack.push_back(&std::get<ListNode>(parent->nested.back()));
            }
        } else {
            bool isBlock = false;
            if (!lineNodes.empty()) {
                if (std::holds_alternative<HeaderNode>(lineNodes[0]) || 
                    std::holds_alternative<QuoteNode>(lineNodes[0]) || 
                    std::holds_alternative<CodeBlockNode>(lineNodes[0]) || 
                    std::holds_alternative<TableNode>(lineNodes[0])) {
                    isBlock = true;
                }
            }
            
            doc.nodes.insert(doc.nodes.end(), lineNodes.begin(), lineNodes.end());
            if (!isBlock && !lineNodes.empty()) {
                doc.nodes.push_back(BlankLineNode{});
            }
            listStack.clear();
        }
    }

    return doc;
}

std::vector<Node> Parser::parseLine(const std::string& line, size_t lineNum) {
    std::string trimmed = trim(line);
    if (trimmed.empty()) return {};

    if (trimmed[0] == '>') {
        int level = 0;
        size_t pos = 0;
        while (pos < trimmed.size() && trimmed[pos] == '>') { level++; pos++; }
        if (level > 3) level = 3;
        bool autoNumbered = false;
        if (pos < trimmed.size() && trimmed[pos] == '#') { autoNumbered = true; pos++; }
        if (pos < trimmed.size() && trimmed[pos] == ' ') {
            std::string content = trimmed.substr(pos + 1);
            if (autoNumbered) {
                headingCounts[level - 1]++;
                for (int i = level; i < 3; ++i) headingCounts[i] = 0;
            }
            return {HeaderNode{level, autoNumbered, autoNumbered ? headingCounts[level - 1] : 0, parseInline(content)}};
        }
    }

    if (trimmed[0] == '.') {
        size_t dots = 0;
        while (dots < trimmed.size() && trimmed[dots] == '.') dots++;
        if (dots > 0 && dots < trimmed.size() && trimmed[dots] == ' ') {
            return {ListNode{ListNode::Type::Unordered, (int)dots, parseInline(trimmed.substr(dots + 1)), {}}};
        }
    }

    if (trimmed.size() >= 2 && std::isdigit(trimmed[0])) {
        size_t pos = 0;
        while (pos < trimmed.size() && std::isdigit(trimmed[pos])) pos++;
        size_t dots = 0;
        while (pos < trimmed.size() && trimmed[pos] == '.') { dots++; pos++; }
        if (dots > 0 && pos < trimmed.size() && trimmed[pos] == ' ') {
            return {ListNode{ListNode::Type::Ordered, (int)dots, parseInline(trimmed.substr(pos + 1)), {}}};
        }
    }

    if (trimmed.size() >= 2 && std::isalpha(trimmed[0])) {
        size_t pos = 0;
        while (pos < trimmed.size() && std::isalpha(trimmed[pos])) pos++;
        size_t dots = 0;
        while (pos < trimmed.size() && trimmed[pos] == '.') { dots++; pos++; }
        if (dots > 0 && pos < trimmed.size() && trimmed[pos] == ' ') {
            return {ListNode{ListNode::Type::Alpha, (int)dots, parseInline(trimmed.substr(pos + 1)), {}}};
        }
    }

    if (trimmed[0] == '`') {
        size_t ticks = 0;
        while (ticks < trimmed.size() && trimmed[ticks] == '`') ticks++;
        if (ticks > 0 && ticks < trimmed.size() && trimmed[ticks] == ' ') {
            return {QuoteNode{static_cast<int>(ticks), parseInline(trimmed.substr(ticks + 1))}};
        }
    }

    if (trimmed == "---") return {LiteralNode{"---"}};

    return parseInline(trimmed);
}

static nlohmann::json serializeNode(const Node& node) {
    return std::visit([](auto&& arg) -> nlohmann::json {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, TextNode>) return {{"type", "Text"}, {"content", arg.content}};
        else if constexpr (std::is_same_v<T, HeaderNode>) {
            nlohmann::json children = nlohmann::json::array();
            for (const auto& child : arg.children) children.push_back(serializeNode(child));
            return {{"type", "Header"}, {"level", arg.level}, {"autoNumbered", arg.autoNumbered}, {"number", arg.number}, {"children", children}};
        } else if constexpr (std::is_same_v<T, ListNode>) {
            std::string typeStr = (arg.type == ListNode::Type::Unordered) ? "Unordered" : (arg.type == ListNode::Type::Ordered ? "Ordered" : "Alpha");
            nlohmann::json content = nlohmann::json::array();
            for (const auto& child : arg.content) content.push_back(serializeNode(child));
            nlohmann::json nested = nlohmann::json::array();
            for (const auto& child : arg.nested) nested.push_back(serializeNode(child));
            return {{"type", "List"}, {"listType", typeStr}, {"level", arg.level}, {"content", content}, {"nested", nested}};
        } else if constexpr (std::is_same_v<T, TableNode>) {
            nlohmann::json rows = nlohmann::json::array();
            for (const auto& row : arg.rows) {
                nlohmann::json rowCells = nlohmann::json::array();
                for (const auto& cell : row) {
                    nlohmann::json cellNodes = nlohmann::json::array();
                    for (const auto& node : cell) cellNodes.push_back(serializeNode(node));
                    rowCells.push_back(cellNodes);
                }
                rows.push_back(rowCells);
            }
            return {{"type", "Table"}, {"hasHeader", arg.hasHeader}, {"rows", rows}};
        } else if constexpr (std::is_same_v<T, QuoteNode>) {
            nlohmann::json children = nlohmann::json::array();
            for (const auto& child : arg.children) children.push_back(serializeNode(child));
            return {{"type", "Quote"}, {"level", arg.level}, {"children", children}};
        } else if constexpr (std::is_same_v<T, LinkNode>) return {{"type", "Link"}, {"url", arg.url}, {"label", arg.label}};
        else if constexpr (std::is_same_v<T, ImageNode>) return {{"type", "Image"}, {"path", arg.path}, {"alt", arg.alt}};
        else if constexpr (std::is_same_v<T, LiteralNode>) return {{"type", "Literal"}, {"content", arg.content}};
        else if constexpr (std::is_same_v<T, CodeBlockNode>) return {{"type", "CodeBlock"}, {"language", arg.language}, {"content", arg.content}};
        else if constexpr (std::is_same_v<T, BoldNode>) {
            nlohmann::json children = nlohmann::json::array();
            for (const auto& child : arg.children) children.push_back(serializeNode(child));
            return {{"type", "Bold"}, {"children", children}};
        } else if constexpr (std::is_same_v<T, ItalicNode>) {
            nlohmann::json children = nlohmann::json::array();
            for (const auto& child : arg.children) children.push_back(serializeNode(child));
            return {{"type", "Italic"}, {"children", children}};
        } else if constexpr (std::is_same_v<T, UnderlineNode>) {
            nlohmann::json children = nlohmann::json::array();
            for (const auto& child : arg.children) children.push_back(serializeNode(child));
            return {{"type", "Underline"}, {"children", children}};
        } else if constexpr (std::is_same_v<T, HighlightNode>) {
            nlohmann::json children = nlohmann::json::array();
            for (const auto& child : arg.children) children.push_back(serializeNode(child));
            return {{"type", "Highlight"}, {"children", children}};
        } else if constexpr (std::is_same_v<T, StrikeNode>) {
            nlohmann::json children = nlohmann::json::array();
            for (const auto& child : arg.children) children.push_back(serializeNode(child));
            return {{"type", "Strike"}, {"children", children}};
        } else if constexpr (std::is_same_v<T, SubscriptNode>) {
            nlohmann::json children = nlohmann::json::array();
            for (const auto& child : arg.children) children.push_back(serializeNode(child));
            return {{"type", "Subscript"}, {"children", children}};
        } else if constexpr (std::is_same_v<T, SuperscriptNode>) {
            nlohmann::json children = nlohmann::json::array();
            for (const auto& child : arg.children) children.push_back(serializeNode(child));
            return {{"type", "Superscript"}, {"children", children}};
        } else if constexpr (std::is_same_v<T, InlineCodeNode>) return {{"type", "InlineCode"}, {"content", arg.content}};
        else if constexpr (std::is_same_v<T, MathInlineNode>) return {{"type", "MathInline"}, {"content", arg.content}};
        else if constexpr (std::is_same_v<T, MathBlockNode>) return {{"type", "MathBlock"}, {"content", arg.content}};
        else if constexpr (std::is_same_v<T, TaskNode>) return {{"type", "Task"}, {"checked", arg.checked}};
        return nlohmann::json::object();
    }, node);
}

nlohmann::json Parser::serialize(const Document& doc) {
    nlohmann::json result = nlohmann::json::object();
    nlohmann::json meta = nlohmann::json::object();
    for (const auto& pair : doc.metadata) {
        meta[pair.first] = pair.second;
    }
    result["metadata"] = meta;
    
    nlohmann::json nodes = nlohmann::json::array();
    for (const auto& node : doc.nodes) nodes.push_back(serializeNode(node));
    result["nodes"] = nodes;
    
    return result;
}

} // namespace solum
