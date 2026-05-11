# Contributing to SOLUM

Thank you for your interest in improving SOLUM! We welcome contributions from developers of all levels. Our goal is to maintain SOLUM as the fastest, most predictable, and most unambiguous markup language available.

## Development Workflow

To ensure the stability and performance of the core parser, please follow these guidelines when contributing.

### 1. Adding a New Formatting Element (Inline)
To add a new inline style (e.g., `~~strikethrough~~`):

1. **AST**: Define a new `Node` struct in `include/ast.hpp` and add it to the `Node` variant.
2. **Parser**: 
   - Update `Parser::parseInline` to recognize the new markers.
   - Update the `handleFormat` logic to instantiate the new `Node` type.
3. **Renderer**: 
   - Update the visitors in `src/renderer.cpp` (both `HtmlRenderer` and `PlainTextRenderer`) to handle the new node type via `std::visit`.
4. **Verification**: Add a comprehensive test case to `tests/test_solum.cpp` and run the test suite.

### 2. Adding a New Block Element (Structural)
To add a new structural element (e.g., a new type of list, a specialized block, or a new header level):

1. **AST**: Add the corresponding node struct and update the `Node` variant in `include/ast.hpp`.
2. **Parser**: Update `Parser::parse` to detect the marker at the start of a line and implement the block's lifecycle (opening, content parsing, and closing).
3. **Renderer**: Implement the rendering logic in `src/renderer.cpp`.

## Contribution Guidelines

### Performance First
The core value proposition of SOLUM is speed.
- **No Regex**: Avoid using regular expressions in the core parsing loop. 
- **Linear Time**: Stick to manual character scanning to maintain $O(n)$ time complexity.
- **Memory Efficiency**: Prefer `std::string_view` over `std::string` where possible to reduce allocations.

### Unambiguity
New markers must not conflict with common technical text. Before proposing a new marker, ensure it does not frequently appear in:
- URLs and file paths.
- C++, Rust, or JavaScript code snippets.
- Mathematical notation.

### Testing Requirements
All changes must be verified using:
1. **The C++ Test Suite**: Run `ctest` or the test executable to ensure no regressions in the AST generation.
2. **WASM Sandbox**: Verify that the changes are correctly reflected in the WASM build and rendered in the browser.

## Getting Started

1. **Fork** the repository and create your feature branch.
2. **Build** the project locally using the instructions in `README.md`.
3. **Implement** your changes and add corresponding tests.
4. **Lint** your code to match the project's style.
5. **Submit** a Pull Request with a clear description of the change and a verification report.
