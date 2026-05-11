# SOLUM Architecture

SOLUM (Strict Organized Line-based Unambiguous Markup) is designed for maximum performance and predictability. It avoids the "magic" and ambiguity of traditional markdown by utilizing a strict, line-based grammar and a manual state-machine parser.

## 1. Pipeline Overview
The processing pipeline follows a strict linear flow:
`SOLUM Text` $\rightarrow$ `Lexical Analysis & Parsing` $\rightarrow$ `AST (Abstract Syntax Tree)` $\rightarrow$ `Renderer` $\rightarrow$ `Target Output (HTML/Text)`

## 2. The Parser (The Core)
The parser is implemented as a manual state machine to guarantee $O(n)$ time complexity.

### Block Parsing
The parser processes the document line-by-line. It first identifies "Block Elements" (Headers, Lists, Quotes, Tables, Code Blocks, Literal Blocks, Math Blocks, and Metadata).
- **Line-Start Constraint**: Block elements are only recognized if their markers appear at the very beginning of the trimmed line.
- **Stack-Based Nesting**: For nested structures like lists and block quotes, a LIFO stack is used to track the current depth and parent-child relationships.

### Inline Parsing
Once a block is identified, its content is passed to the inline parser.
- **Linear Scanning**: The inline parser scans the text character-by-character.
- **Symmetry-based LIFO**: Formatting markers (e.g., `!!`, `::`) are handled using a stack. This allows for complex nesting and, crucially, **intersecting styles**.
- **Intersection Handling**: If a closing marker is found that doesn't match the current top of the stack, the parser "collapses" the intervening styles, wraps them in their respective nodes, and re-opens them to maintain style continuity.

## 3. The AST (Abstract Syntax Tree)
The AST is implemented using `std::variant` for type safety and performance, avoiding the overhead of virtual function calls.

- **Node Variant**: A `Node` can be any of the defined structural elements (e.g., `TextNode`, `BoldNode`, `ListNode`).
- **Recursive Structure**: Nodes like `BoldNode` or `ListNode` contain a `std::vector<Node>`, allowing for arbitrary nesting.
- **Document Root**: A `Document` consists of a metadata map and a sequence of top-level `Node`s.

## 4. The Renderer
The renderer is decoupled from the parser via the `IRenderer` interface.
- **Visitor Pattern**: The renderer uses `std::visit` to traverse the AST.
- **Environment Awareness**: Through `RenderConfig`, the renderer can resolve assets (links/images) based on the host environment (e.g., adding a CDN prefix).
- **Multiple Targets**: Because the AST is generic, different renderers (HTML, Plain Text, LaTeX) can be implemented without changing the parser.
