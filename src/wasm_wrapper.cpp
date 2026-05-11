#include <emscripten/emscripten.h>
#include <string>
#include "parser.hpp"
#include "renderer.hpp"
#include <nlohmann/json.hpp>

extern "C" {

// Renders SOLUM text directly to HTML
EMSCRIPTEN_KEEPALIVE
const char* solum_render(const char* input) {
    solum::Parser parser;
    auto ast = parser.parse(std::string(input));
    
    solum::RenderConfig config;
    config.resolveAsset = [](const std::string& p) { return p; };
    
    // We use a static string or a managed buffer to return the result to JS
    static std::string result;
    result = solum::SolumRenderer::render(ast, config);
    return result.c_str();
}

// Serializes SOLUM text to JSON AST
EMSCRIPTEN_KEEPALIVE
const char* solum_serialize(const char* input) {
    solum::Parser parser;
    auto ast = parser.parse(std::string(input));
    
    static std::string result;
    result = parser.serialize(ast).dump();
    return result.c_str();
}

// Renders SOLUM text to Plain Text
EMSCRIPTEN_KEEPALIVE
const char* solum_render_text(const char* input) {
    solum::Parser parser;
    auto ast = parser.parse(std::string(input));
    
    solum::RenderConfig config;
    config.resolveAsset = [](const std::string& p) { return p; };
    
    static std::string result;
    solum::PlainTextRenderer renderer;
    result = renderer.render(ast, config);
    return result.c_str();
}

}
