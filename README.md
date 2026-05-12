# SOLUM (Strict Organized Line-based Unambiguous Markup)

SOLUM is a high-performance, unambiguous markup language designed to eliminate the complexity and "guessing game" of traditional markdown. It is implemented as a C++17 library (`libsolum`) and provided as a WebAssembly module for seamless web integration.

## Key Features

- **Unambiguous Parsing**: No "magic" rules or context-dependent behavior. Symbols have strict, predictable meaning.
- **Extreme Performance**: A manual state-machine parser ensures linear $O(n)$ time complexity and minimal memory overhead.
- **Intersecting Styles**: Unlike standard Markdown, SOLUM handles overlapping inline styles (e.g., `!!Bold ::Both!! Italic::`) gracefully.
- **Decoupled Architecture**: Clear separation between the Parser (Text $\rightarrow$ AST) and the Renderer (AST $\rightarrow$ Target), allowing for multiple output formats (HTML, LaTeX, Plain Text).
- **Web-Ready**: First-class support for WebAssembly, enabling the same high-performance parser to run in the browser.

## Documentation

Comprehensive guides are available in the `docs/` directory:

- [**Syntax Guide**](docs/syntax.md) - Exhaustive reference of all markers, block elements, and inline rules.
- [**Integration Guide**](docs/integration.md) - How to integrate the C++ library or WASM module into your project.
- [**Architecture Overview**](ARCHITECTURE.md) - Technical deep-dive into the state-machine parser and AST design.
- [**Contributing Guide**](CONTRIBUTING.md) - Guidelines for adding new features and maintaining performance.

## Quick Start

### C++ Integration
```cpp
#include "parser.hpp"
#include "renderer.hpp"

solum::Parser parser;
solum::Document doc = parser.parse("Your SOLUM text here");
std::string html = solum::SolumRenderer::render(doc, {});
```

### WebAssembly / JavaScript
```javascript
import SolumModule from './solum.js';
SolumModule().then(module => {
    const html = module._solum_render("Your SOLUM text here");
    console.log(html);
});
```

### CLI Tool
```bash
./solum_cli input.sm > output.html
```

### Showcase Playground
A live, interactive playground is available in the `showcase/` directory or at [https://pomilon.github.io/SOLUM/](https://pomilon.github.io/SOLUM/). It allows you to test SOLUM in real-time and view the HTML and JSON AST outputs directly in your browser.

## Build Instructions

### C++ Library
```bash
mkdir build && cd build
cmake ..
make
```

### WASM Module
Requires [Emscripten](https://emscripten.org/).
```bash
emcc -O3 src/parser.cpp src/renderer.cpp src/wasm_wrapper.cpp \
     -I include -I third_party \
     -s EXPORTED_FUNCTIONS='["_solum_render", "_solum_serialize"]' \
     -s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' \
     -s MODULARIZE=1 -s EXPORT_NAME="SolumModule" \
     -o solum.js
```
