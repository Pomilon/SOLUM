#include <gtest/gtest.h>
#include "parser.hpp"
#include "renderer.hpp"

using namespace solum;

RenderConfig DefaultConfig() {
    RenderConfig config;
    config.resolveAsset = [](const std::string& p) { return p; };
    return config;
}

TEST(SolumParser, BasicHeaders) {
    Parser parser;
    std::string input = "> Header 1\n>> Header 2\n>>> Header 3";
    auto doc = parser.parse(input);
    std::string html = SolumRenderer::render(doc, DefaultConfig());
    EXPECT_EQ(html, "<h1>Header 1</h1><h2>Header 2</h2><h3>Header 3</h3>");
}

TEST(SolumParser, Lists) {
    Parser parser;
    std::string input = ". Item 1\n1. Item 2\na. Item 3";
    auto doc = parser.parse(input);
    std::string html = SolumRenderer::render(doc, DefaultConfig());
    EXPECT_EQ(html, "<ul><li>Item 1</li></ul><ol><li>Item 2</li></ol><ol type='a'><li>Item 3</li></ol>");
}

TEST(SolumParser, TaskLists) {
    Parser parser;
    std::string input = ". [ ] Task 1\n. [x] Task 2";
    auto doc = parser.parse(input);
    std::string html = SolumRenderer::render(doc, DefaultConfig());
    EXPECT_EQ(html, "<ul><li><input type='checkbox' disabled> Task 1</li><li><input type='checkbox' checked disabled> Task 2</li></ul>");
}

TEST(SolumParser, Math) {
    Parser parser;
    std::string input = "Inline $$E=mc^2$$ and block\n$$$\nC = a^2 + b^2\n$$$\nEnd";
    auto doc = parser.parse(input);
    std::string html = SolumRenderer::render(doc, DefaultConfig());
    EXPECT_EQ(html, "Inline <span class=\"math-inline\">E=mc^2</span> and block<br/><div class=\"math-block\">C = a^2 + b^2\n</div>End<br/>");
}

TEST(SolumParser, Metadata) {
    Parser parser;
    std::string input = "===META\nTitle: Test\nAuthor: Solum\n===\nContent";
    auto doc = parser.parse(input);
    std::string html = SolumRenderer::render(doc, DefaultConfig());
    EXPECT_EQ(html, "<header><strong>Title:</strong> Test<br/><strong>Author:</strong> Solum<br/></header>\nContent<br/>");
}

TEST(SolumParser, QuoteBlock) {
    Parser parser;
    std::string input = "[q]\nThis is a quote\n[q]";
    auto doc = parser.parse(input);
    std::string html = SolumRenderer::render(doc, DefaultConfig());
    EXPECT_EQ(html, "<blockquote>This is a quote<br/></blockquote>");
}

TEST(SolumParser, InlineFormatting) {
    Parser parser;
    std::string input = "!!Bold!! and ::Italics:: and ==Underline== and %%Highlight%% and {Code}";
    auto doc = parser.parse(input);
    std::string html = SolumRenderer::render(doc, DefaultConfig());
    EXPECT_EQ(html, "<strong>Bold</strong> and <em>Italics</em> and <u>Underline</u> and <mark>Highlight</mark> and <code>Code</code><br/>");
}

TEST(SolumParser, NestedFormatting) {
    Parser parser;
    std::string input = "!!Bold and ::Italic::!!";
    auto doc = parser.parse(input);
    std::string html = SolumRenderer::render(doc, DefaultConfig());
    EXPECT_EQ(html, "<strong>Bold and <em>Italic</em></strong><br/>");
}

TEST(SolumParser, Shields) {
    Parser parser;
    std::string input = "Normal !!Bold!! and ''Literal !!Not Bold!!''";
    auto doc = parser.parse(input);
    std::string html = SolumRenderer::render(doc, DefaultConfig());
    EXPECT_EQ(html, "Normal <strong>Bold</strong> and <pre>Literal !!Not Bold!!</pre><br/>");
}

TEST(SolumParser, BlockShield) {
    Parser parser;
    std::string input = "Text\n===\nRaw block\n++Not Bold++\n===\nText";
    auto doc = parser.parse(input);
    std::string html = SolumRenderer::render(doc, DefaultConfig());
    EXPECT_EQ(html, "Text<br/><pre>Raw block\n++Not Bold++\n</pre>Text<br/>");
}

TEST(SolumParser, Assets) {
    Parser parser;
    std::string input = "[@ https://google.com \"Google\"] and [! ./img.png \"Alt\"]";
    auto doc = parser.parse(input);
    
    RenderConfig config;
    config.resolveAsset = [](const std::string& p) -> std::string { 
        if (p == "./img.png") return "http://cdn.com/img.png";
        return p; 
    };
    
    std::string html = SolumRenderer::render(doc, config);
    EXPECT_EQ(html, "<a href=\"https://google.com\">Google</a> and <img src=\"http://cdn.com/img.png\" alt=\"Alt\"><br/>");
}

TEST(SolumParser, Tables) {
    Parser parser;
    std::string input = "[t]\nHeader 1 || Header 2\n---\nCell 1 || Cell 2\n[t]";
    auto doc = parser.parse(input);
    std::string html = SolumRenderer::render(doc, DefaultConfig());
    EXPECT_EQ(html, "<table><thead><tr><th>Header 1</th><th>Header 2</th></tr></thead><tbody><tr><td>Cell 1</td><td>Cell 2</td></tr></tbody></table>");
}

TEST(SolumParser, PlainText) {
    Parser parser;
    std::string input = "> Header\n. Item 1\n!!Bold!! and $$E=mc^2$$\n$$$\nC = a^2 + b^2\n$$$";
    auto doc = parser.parse(input);
    PlainTextRenderer renderer;
    std::string text = renderer.render(doc, DefaultConfig());
    EXPECT_EQ(text, "\nHeader\n- Item 1\nBold and $$E=mc^2$$\n\n$$$\nC = a^2 + b^2\n$$$\n");
}

TEST(SolumParser, MarkerCollisions) {
    Parser parser;
    // Check that block markers appearing inline are treated as literal text
    std::string input = "Block markers like {{cpp: }}, ===, and $$$ should be literal here.";
    auto doc = parser.parse(input);
    std::string html = SolumRenderer::render(doc, DefaultConfig());
    EXPECT_EQ(html, "Block markers like {{cpp: }}, ===, and $$$ should be literal here.<br/>");
}

TEST(SolumParser, Subscript) {
    Parser parser;
    std::string input = "H,,2,,O";
    auto doc = parser.parse(input);
    std::string html = SolumRenderer::render(doc, DefaultConfig());
    EXPECT_EQ(html, "H<sub>2</sub>O<br/>");
}

TEST(SolumParser, Superscript) {
    Parser parser;
    std::string input = "x^^2^^";
    auto doc = parser.parse(input);
    std::string html = SolumRenderer::render(doc, DefaultConfig());
    EXPECT_EQ(html, "x<sup>2</sup><br/>");
}

TEST(SolumParser, SubscriptSuperscriptNested) {
    Parser parser;
    std::string input = ",,sub!!bold^^sup^^!!,,";
    auto doc = parser.parse(input);
    std::string html = SolumRenderer::render(doc, DefaultConfig());
    EXPECT_EQ(html, "<sub>sub<strong>bold<sup>sup</sup></strong></sub><br/>");
}

TEST(SolumParser, SuperscriptSubscriptNested) {
    Parser parser;
    std::string input = "^^sup,,sub!!both!!,,^^";
    auto doc = parser.parse(input);
    std::string html = SolumRenderer::render(doc, DefaultConfig());
    EXPECT_EQ(html, "<sup>sup<sub>sub<strong>both</strong></sub></sup><br/>");
}

TEST(SolumParser, SubscriptInPlainText) {
    Parser parser;
    std::string input = "H,,2,,O";
    auto doc = parser.parse(input);
    PlainTextRenderer renderer;
    std::string text = renderer.render(doc, DefaultConfig());
    EXPECT_EQ(text, "H,,2,,O\n");
}

TEST(SolumParser, SubscriptEscaped) {
    Parser parser;
    std::string input = "\\,,not subscript\\,,";
    auto doc = parser.parse(input);
    std::string html = SolumRenderer::render(doc, DefaultConfig());
    EXPECT_EQ(html, ",,not subscript,,<br/>");
}

TEST(SolumParser, UnclosedMarkers) {
    Parser parser;
    std::string input = "!!Unclosed bold";
    auto doc = parser.parse(input);
    std::string html = SolumRenderer::render(doc, DefaultConfig());
    EXPECT_EQ(html, "!!Unclosed bold<br/>");
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
