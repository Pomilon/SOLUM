#include "parser.hpp"
#include "renderer.hpp"
#include <iostream>
#include <string>
#include <vector>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Convert raw data to string
    std::string input(reinterpret_cast<const char*>(data), size);
    
    try {
        solum::Parser parser;
        // Exercise the parser
        auto doc = parser.parse(input);
        
        // Exercise the serializer
        auto json = parser.serialize(doc);
        
        // Exercise the renderer (HTML)
        solum::RenderConfig config;
        config.resolveAsset = [](const std::string& p) { return p; };
        std::string html = solum::SolumRenderer::render(doc, config);
        
    } catch (...) {
        // We want to find crashes, not expected exceptions.
        // However, the parser is designed not to throw.
    }
    
    return 0;
}
