# Integration Guide

This guide explains how to integrate `libsolum` into your projects, whether you are building a native C++ application or a web-based tool.

## 1. C++ Library Integration

`libsolum` is a header-only compatible C++17 library (though it is typically built as a static library for faster compilation).

### Build and Install
Build the library using CMake:
```bash
mkdir build && cd build
cmake ..
make
```
This produces `libsolum.a` (on Linux/macOS) in the `build` directory.

### Basic Usage
The integration follows a simple two-step process: **Parse** $\rightarrow$ **Render**.

```cpp
#include "parser.hpp"
#include "renderer.hpp"
#include <iostream>

int main() {
    // 1. Initialize the parser and process SOLUM text into an AST
    solum::Parser parser;
    std::string input = "> Hello SOLUM\n. Item 1\n. Item 2";
    solum::Document doc = parser.parse(input);

    // 2. Configure asset resolution (optional)
    // This callback is used by the renderer to resolve relative paths for links/images
    solum::RenderConfig config;
    config.resolveAsset = [](const std::string& path) {
        return "https://cdn.example.com/assets/" + path;
    };

    // 3. Render the AST to HTML
    std::string html = solum::SolumRenderer::render(doc, config);
    
    std::cout << html << std::endl;
    return 0;
}
```

## 2. WebAssembly Integration

SOLUM provides a high-performance WASM module, allowing you to parse and render SOLUM text directly in the browser with near-native speed.

### Setup
You will need the following files generated from the Emscripten build:
- `solum.js` (The JS glue code)
- `solum.wasm` (The compiled binary)

### JavaScript Usage
The WASM module is modularized for easy integration into modern JS frameworks.

```javascript
import SolumModule from './solum.js';

async function initSolum() {
    const module = await SolumModule();
    
    const solumText = `
    > Welcome to SOLUM
    !!This is bold and ::this is both!!::
    
    . Feature 1
    . Feature 2
    `;

    // _solum_render is the primary exported function
    const html = module._solum_render(solumText);
    document.getElementById('output').innerHTML = html;
}

initSolum().catch(console.error);
```

## 3. CLI Tool Usage

The `solum_cli` is a standalone utility for static site generation, documentation builds, or local previews.

### Basic Conversion
```bash
./solum_cli input.sm > output.html
```

### Batch Processing
You can easily integrate the CLI into a build script:
```bash
for f in docs/*.sm; do 
    ./solum_cli "$f" > "dist/${f%.sm}.html"; 
done
```
