#include "renderer.hpp"
#include <sstream>

namespace solum {

// --- Internal Helpers for HTML Rendering ---
namespace {
    std::string escapeHtml(const std::string& data) {
        std::string buffer;
        buffer.reserve(data.size());
        for (size_t pos = 0; pos != data.size(); ++pos) {
            switch (data[pos]) {
                case '&':  buffer.append("&amp;");       break;
                case '\"': buffer.append("&quot;");      break;
                case '\'': buffer.append("&apos;");      break;
                case '<':  buffer.append("&lt;");        break;
                case '>':  buffer.append("&gt;");        break;
                default:   buffer.append(1, data[pos]); break;
            }
        }
        return buffer;
    }

    std::string htmlRenderNode(const Node& node, const RenderConfig& config);

    std::string htmlRenderNodeSequence(const std::vector<Node>& nodes, const RenderConfig& config) {
        std::string result;
        for (size_t i = 0; i < nodes.size(); ++i) {
            const auto& node = nodes[i];
            if (std::holds_alternative<ListNode>(node)) {
                auto firstList = std::get<ListNode>(node);
                std::string openTag = "ul";
                std::string closeTag = "ul";
                if (firstList.type == ListNode::Type::Ordered) {
                    openTag = "ol";
                    closeTag = "ol";
                } else if (firstList.type == ListNode::Type::Alpha) {
                    openTag = "ol type='a'";
                    closeTag = "ol";
                }

            std::string listContent;
            while (i < nodes.size() && std::holds_alternative<ListNode>(nodes[i])) {
                auto listNode = std::get<ListNode>(nodes[i]);
                if (listNode.type != firstList.type) break;
                
                std::string itemContent = htmlRenderNodeSequence(listNode.content, config);
                std::string nestedContent = htmlRenderNodeSequence(listNode.nested, config);
                listContent += "<li>" + itemContent + nestedContent + "</li>";
                i++;
            }
            result += "<" + openTag + ">" + listContent + "</" + closeTag + ">";
            i--; 

            } else {
                result += htmlRenderNode(node, config);
            }
        }
        return result;
    }

    std::string htmlRenderNode(const Node& node, const RenderConfig& config) {
        return std::visit([&](auto&& arg) -> std::string {
            using T = std::decay_t<decltype(arg)>;
            
            if constexpr (std::is_same_v<T, TextNode>) {
                return escapeHtml(arg.content);
            } 
            else if constexpr (std::is_same_v<T, BlankLineNode>) {
                return "<br/>";
            }
            else if constexpr (std::is_same_v<T, HeaderNode>) {
                std::string tag = "h" + std::to_string(arg.level);
                std::string prefix = arg.autoNumbered ? std::to_string(arg.number) + ". " : "";
                return "<" + tag + ">" + prefix + htmlRenderNodeSequence(arg.children, config) + "</" + tag + ">";
            }
            else if constexpr (std::is_same_v<T, ListNode>) {
                return htmlRenderNodeSequence({arg}, config);
            }
            else if constexpr (std::is_same_v<T, QuoteNode>) {
                std::string html;
                for (int i = 0; i < arg.level; ++i) html += "<blockquote>";
                html += htmlRenderNodeSequence(arg.children, config);
                for (int i = 0; i < arg.level; ++i) html += "</blockquote>";
                return html;
            }
            else if constexpr (std::is_same_v<T, TableNode>) {
                std::string html = "<table>";
                if (arg.hasHeader && !arg.rows.empty()) {
                    html += "<thead><tr>";
                    for (const auto& cell : arg.rows[0]) {
                        html += "<th>" + htmlRenderNodeSequence(cell, config) + "</th>";
                    }
                    html += "</tr></thead>";
                }
                html += "<tbody>";
                size_t startRow = arg.hasHeader ? 1 : 0;
                for (size_t i = startRow; i < arg.rows.size(); ++i) {
                    html += "<tr>";
                    for (const auto& cell : arg.rows[i]) {
                        html += "<td>" + htmlRenderNodeSequence(cell, config) + "</td>";
                    }
                    html += "</tr>";
                }
                html += "</tbody></table>";
                return html;
            }
            else if constexpr (std::is_same_v<T, MathInlineNode>) {
                return "<span class=\"math-inline\">" + escapeHtml(arg.content) + "</span>";
            }
            else if constexpr (std::is_same_v<T, MathBlockNode>) {
                return "<div class=\"math-block\">" + escapeHtml(arg.content) + "</div>";
            }
            else if constexpr (std::is_same_v<T, TaskNode>) {
                return "<input type='checkbox' " + std::string(arg.checked ? "checked " : "") + "disabled>";
            }
            else if constexpr (std::is_same_v<T, LinkNode>) {
                return "<a href=\"" + escapeHtml(config.resolveAsset(arg.url)) + "\">" + escapeHtml(arg.label) + "</a>";
            }

            else if constexpr (std::is_same_v<T, ImageNode>) {
                return "<img src=\"" + escapeHtml(config.resolveAsset(arg.path)) + "\" alt=\"" + escapeHtml(arg.alt) + "\">";
            }
            else if constexpr (std::is_same_v<T, LiteralNode>) {
                return "<pre>" + escapeHtml(arg.content) + "</pre>";
            }
            else if constexpr (std::is_same_v<T, CodeBlockNode>) {
                return "<pre><code class=\"language-" + escapeHtml(arg.language) + "\">" + escapeHtml(arg.content) + "</code></pre>";
            }
            else if constexpr (std::is_same_v<T, BoldNode>) {
                return "<strong>" + htmlRenderNodeSequence(arg.children, config) + "</strong>";
            }
            else if constexpr (std::is_same_v<T, ItalicNode>) {
                return "<em>" + htmlRenderNodeSequence(arg.children, config) + "</em>";
            }
            else if constexpr (std::is_same_v<T, UnderlineNode>) {
                return "<u>" + htmlRenderNodeSequence(arg.children, config) + "</u>";
            }
        else if constexpr (std::is_same_v<T, HighlightNode>) {
            return "<mark>" + htmlRenderNodeSequence(arg.children, config) + "</mark>";
        }
        else if constexpr (std::is_same_v<T, StrikeNode>) {
            return "<s>" + htmlRenderNodeSequence(arg.children, config) + "</s>";
        }
        else if constexpr (std::is_same_v<T, SubscriptNode>) {
            return "<sub>" + htmlRenderNodeSequence(arg.children, config) + "</sub>";
        }
        else if constexpr (std::is_same_v<T, SuperscriptNode>) {
            return "<sup>" + htmlRenderNodeSequence(arg.children, config) + "</sup>";
        }
        else if constexpr (std::is_same_v<T, InlineCodeNode>) {
            return "<code>" + escapeHtml(arg.content) + "</code>";
        }

            return "";
        }, node);
    }
}

std::string HtmlRenderer::render(const Document& doc, const RenderConfig& config) {
    std::string html;
    if (!doc.metadata.empty()) {
        html += "<header>";
        for (const auto& pair : doc.metadata) {
            html += "<strong>" + escapeHtml(pair.first) + ":</strong> " + escapeHtml(pair.second) + "<br/>";
        }
        html += "</header>\n";
    }
    html += htmlRenderNodeSequence(doc.nodes, config);
    return html;
}


// --- Plain Text Rendering Logic ---
namespace {
    std::string textRenderNode(const Node& node, const RenderConfig& config);

    std::string textRenderNodeSequence(const std::vector<Node>& nodes, const RenderConfig& config) {
        std::string result;
        for (const auto& node : nodes) {
            result += textRenderNode(node, config);
        }
        return result;
    }

    std::string textRenderNode(const Node& node, const RenderConfig& config) {
        return std::visit([&](auto&& arg) -> std::string {
            using T = std::decay_t<decltype(arg)>;
            
            if constexpr (std::is_same_v<T, TextNode>) {
                return arg.content;
            } 
            else if constexpr (std::is_same_v<T, BlankLineNode>) {
                return "\n";
            }
            else if constexpr (std::is_same_v<T, HeaderNode>) {
                std::string prefix = arg.autoNumbered ? std::to_string(arg.number) + ". " : "";
                return "\n" + prefix + textRenderNodeSequence(arg.children, config) + "\n";
            }
            else if constexpr (std::is_same_v<T, ListNode>) {
                std::string prefix = (arg.type == ListNode::Type::Unordered) ? "- " : (arg.type == ListNode::Type::Ordered ? std::to_string(arg.level) + ". " : "a. ");
                return prefix + textRenderNodeSequence(arg.content, config) + "\n" + textRenderNodeSequence(arg.nested, config);
            }
            else if constexpr (std::is_same_v<T, QuoteNode>) {
                std::string prefix(arg.level * 4, ' ');
                return "\n" + prefix + textRenderNodeSequence(arg.children, config) + "\n";
            }
            else if constexpr (std::is_same_v<T, TableNode>) {
                return "[Table]\n";
            }
            else if constexpr (std::is_same_v<T, TaskNode>) {
                return (arg.checked ? "[x] " : "[ ] ");
            }
            else if constexpr (std::is_same_v<T, LinkNode>) {
                return "[" + arg.label + "](" + arg.url + ")";
            }

            else if constexpr (std::is_same_v<T, ImageNode>) {
                return "[Image: " + arg.alt + "]";
            }
            else if constexpr (std::is_same_v<T, LiteralNode>) {
                return "\n" + arg.content + "\n";
            }
            else if constexpr (std::is_same_v<T, CodeBlockNode>) {
                return "\n--- Code (" + arg.language + ") ---\n" + arg.content + "\n----------------\n";
            }
            else if constexpr (std::is_same_v<T, BoldNode>) {
                return textRenderNodeSequence(arg.children, config);
            }
            else if constexpr (std::is_same_v<T, ItalicNode>) {
                return textRenderNodeSequence(arg.children, config);
            }
            else if constexpr (std::is_same_v<T, UnderlineNode>) {
                return textRenderNodeSequence(arg.children, config);
            }
        else if constexpr (std::is_same_v<T, HighlightNode>) {
            return textRenderNodeSequence(arg.children, config);
        }
        else if constexpr (std::is_same_v<T, StrikeNode>) {
            return "~~" + textRenderNodeSequence(arg.children, config) + "~~";
        }
        else if constexpr (std::is_same_v<T, SubscriptNode>) {
            return ",," + textRenderNodeSequence(arg.children, config) + ",,";
        }
        else if constexpr (std::is_same_v<T, SuperscriptNode>) {
            return "^^" + textRenderNodeSequence(arg.children, config) + "^^";
        }
        else if constexpr (std::is_same_v<T, InlineCodeNode>) {
        return "`" + arg.content + "`";
        }
        else if constexpr (std::is_same_v<T, MathInlineNode>) {
        return "$$" + arg.content + "$$";
        }
        else if constexpr (std::is_same_v<T, MathBlockNode>) {
        return "\n$$$\n" + arg.content + "$$$\n";
        }

        return "";

        }, node);
    }
}

std::string PlainTextRenderer::render(const Document& doc, const RenderConfig& config) {
    std::string text;
    if (!doc.metadata.empty()) {
        for (const auto& pair : doc.metadata) {
            text += pair.first + ": " + pair.second + "\n";
        }
        text += "------------------\n";
    }
    text += textRenderNodeSequence(doc.nodes, config);
    return text;
}

} // namespace solum
