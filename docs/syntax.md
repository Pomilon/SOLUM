# SOLUM Syntax Guide

This document provides an exhaustive reference for the SOLUM markup language. SOLUM is designed to be unambiguous: markers are either recognized in their strict positions or treated as literal text.

## 1. Block Elements

Block elements must start at the very beginning of a line (after optional whitespace). **Crucially, most block markers must be followed by a space to be recognized.**

### Headers
Headers are defined by the `>` symbol. 

| SOLUM | HTML | Description |
| :--- | :--- | :--- |
| `> Header` | `<h1>` | Level 1 Header |
| `>> Header` | `<h2>` | Level 2 Header |
| `>>> Header` | `<h3>` | Level 3 Header |
| `># Header` | `<h1>` (numbered) | Auto-numbered Header (1., 2., etc.) |

*Note: A space is required after the markers (e.g., `>Header` will be treated as literal text).*

### Lists
Lists support Unordered, Ordered, and Alpha types. Nesting is achieved by increasing the number of dots.

| Type | Level 1 | Level 2 | Level 3 |
| :--- | :--- | :--- | :--- |
| **Unordered** | `. Item` | `.. Item` | `... Item` |
| **Ordered** | `1. Item` | `1.. Item` | `1... Item` |
| **Alpha** | `a. Item` | `a.. Item` | `a... Item` |

*Note: A space is required after the dots. For Ordered and Alpha lists, the marker is a digit/letter followed by one or more dots.*

### Quotes
- **Block Quotes**: Enclose content between `[q]` markers. These can be nested.
  `[q]`
  `This is a block quote.`
  `[q]`
- **Line Quotes**: Use stacked backticks at the start of a line, followed by a space.
  `` ` Level 1 Quote ``
  `` `` Level 2 Quote ``

### Tables
Tables are enclosed in `[t]` markers. Cells are separated by `||`.

`[t]`
`Name || Age || City`
`---`
`Alice || 30 || New York`
`Bob || 25 || London`
`[t]`

### Specialized Blocks
- **Code Blocks**: `{{lang: ... }}`. The `lang` prefix (followed by a colon) specifies the language for syntax highlighting (e.g., `{{cpp:`, `{{js:`, `{{py:`). Ends with `}}` on a new line.
- **Literal Blocks**: `=== ... ===`. Disables all parsing; content is rendered exactly as written.
- **Math Blocks**: `$$$ ... $$$`. Used for multi-line mathematical formulas.
- **Metadata**: Must appear at the very top of the file. It starts with a line containing exactly `===META` and ends with a line containing exactly `===`.
  `===META`
  `Title: My Document`
  `Author: John Doe`
  `===`

## 2. Inline Elements

Inline elements can be used anywhere in the document.

### Formatting

| Effect | Marker | Example | Result |
| :--- | :--- | :--- | :--- |
| **Bold** | `!!` | `!!Bold!!` | **Bold** |
| **Italic** | `::` | `::Italic::` | *Italic* |
| **Underline** | `==` | `==Underline==` | <u>Underline</u> |
| **Highlight** | `%%` | `%%Highlight%%` | <mark>Highlight</mark> |
| **Strike** | `~~` | `~~Strike~~` | ~~Strike~~ |
| **Inline Code** | `{}` | `{int x = 0;}` | `int x = 0;` |
| **Inline Literal**| `''` | `''literal''` | Literal text |

### Assets and Math
- **Links**: `[@ url "label"]` $\rightarrow$ `<a href="url">label</a>`
- **Images**: `[! path "alt"]` $\rightarrow$ `<img src="path" alt="alt">`
- **Inline Math**: `$$formula$$` $\rightarrow$ Rendered as inline math.

### Tasks
Task markers `[ ]` (unchecked) and `[x]` (checked) can be placed anywhere inline.

### Escaping
To render a marker literally, use the backslash `\` escape character. The character immediately following the backslash is treated as literal text.
- `\!!` $\rightarrow$ `!!`
- `\>` $\rightarrow$ `>`
- `\#` $\rightarrow$ `#`

## 3. Advanced Logic

### Intersecting Styles
SOLUM handles overlapping styles by preserving the inner style and wrapping the intersection.

**Input**: `!!Bold ::Both!! Italic::`
**Rendered**: **Bold *Both*** *Italic*

### Unclosed Markers
To prevent "leaking" styles, any marker that is not properly closed within the same block is treated as raw text.
`!!This is not bold` $\rightarrow$ renders as `!!This is not bold`.
