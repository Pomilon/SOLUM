#pragma once

#include <string>
#include <vector>
#include <variant>
#include <memory>

namespace solum {

struct HeaderNode;
struct ListNode;
struct TableNode;
struct QuoteNode;
struct LinkNode;
struct ImageNode;
struct LiteralNode;
struct CodeBlockNode;
struct TextNode;
struct BlankLineNode;
struct BoldNode;
struct ItalicNode;
struct UnderlineNode;
struct HighlightNode;
struct StrikeNode;
struct InlineCodeNode;
struct MathInlineNode;
struct MathBlockNode;
struct TaskNode;

using Node = std::variant<
    TextNode, 
    HeaderNode, 
    ListNode, 
    TableNode, 
    QuoteNode, 
    LinkNode, 
    ImageNode, 
    LiteralNode, 
    CodeBlockNode,
    BlankLineNode,
    BoldNode,
    ItalicNode,
    UnderlineNode,
    HighlightNode,
    StrikeNode,
    InlineCodeNode,
    MathInlineNode,
    MathBlockNode,
    TaskNode
>;

struct TextNode {
    std::string content;
};

struct BlankLineNode {};

struct HeaderNode {
    int level;
    bool autoNumbered;
    int number = 0;
    std::vector<Node> children;
};

struct ListNode {
    enum class Type { Unordered, Ordered, Alpha };
    Type type;
    int level = 1;
    std::vector<Node> content;
    std::vector<Node> nested;
};

struct TaskNode {
    bool checked;
};

struct BoldNode {
    std::vector<Node> children;
};

struct ItalicNode {
    std::vector<Node> children;
};

struct UnderlineNode {
    std::vector<Node> children;
};

struct HighlightNode {
    std::vector<Node> children;
};

struct StrikeNode {
    std::vector<Node> children;
};

struct InlineCodeNode {
    std::string content;
};

struct MathInlineNode {
    std::string content;
};

struct MathBlockNode {
    std::string content;
};

struct TableNode {
    bool hasHeader;
    std::vector<std::vector<std::vector<Node>>> rows;
};

struct QuoteNode {
    int level = 1;
    std::vector<Node> children;
};

struct LinkNode {
    std::string url;
    std::string label;
};

struct ImageNode {
    std::string path;
    std::string alt;
};

struct LiteralNode {
    std::string content;
};

struct CodeBlockNode {
    std::string language;
    std::string content;
};

struct Document {
    std::vector<std::pair<std::string, std::string>> metadata;
    std::vector<Node> nodes;
};

} // namespace solum



