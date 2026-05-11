#pragma once

#include "ast.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>

namespace solum {

struct RenderConfig {
    std::function<std::string(const std::string& path)> resolveAsset;
};

class IRenderer {
public:
    virtual ~IRenderer() = default;
    virtual std::string render(const Document& doc, const RenderConfig& config) = 0;
};

class HtmlRenderer : public IRenderer {
public:
    std::string render(const Document& doc, const RenderConfig& config) override;
};

class PlainTextRenderer : public IRenderer {
public:
    std::string render(const Document& doc, const RenderConfig& config) override;
};

class SolumRenderer {
public:
    static std::string render(const Document& doc, const RenderConfig& config) {
        HtmlRenderer renderer;
        return renderer.render(doc, config);
    }
};

} // namespace solum
