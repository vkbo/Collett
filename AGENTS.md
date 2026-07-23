# AGENTS.md

Guidance for AI coding agents working in this repository.

## Project

Collett is a rich-text novel-writing editor written in C++/Qt6. It is an alternative
implementation of [novelWriter](https://github.com/vkbo/novelWriter), using a Qt6 rich
text document format instead of novelWriter's plain-text markdown files. The project is
experimental and not yet a usable application.

Reference sources, useful when porting behavior or checking API details:
- `~/Code/novelWriter/Source` — the Python reference implementation whose features and
  behavior Collett aims to mirror.
- `~/Code/CollettArchived` — an earlier C++ implementation of Collett. Only a partial
  reference: it used a single flat JSON document, not the current per-node/handle
  content-file model.
- `~/Source/Qt6Source` — local Qt6 source checkout, for looking up exact Qt6 API
  behavior (QTextDocument, QTextCursor, model/view classes, etc.) instead of guessing.

## Build

```sh
cmake -S . -B build
cmake --build build -j$(nproc)
```

Requires Qt6 (Core, Widgets, Svg, LinguistTools). The built binary is `build/collett`.

## Code Conventions

- License header block at the top of every source/header file (copy the format from an
  existing file in the same directory).
- Formatting follows `.clang-format` (LLVM-based, 4-space indent, no tabs).
- Doxygen-style comments (`/**! @brief ... */`) on non-trivial new functions and methods,
  placed above the definition in the `.cpp` file (not the header declaration). Skip them
  for trivial one-line accessors/mutators. See `Tree::newHandle()` and `Tree::isHandle()`
  in `src/project/tree.cpp` for the expected style.
- Item/node handles are 13-character lowercase hex strings (52-bit random), not UUIDs.
  Generate new ones with `Tree::newHandle()` and validate untrusted/loaded ones with
  `Tree::isHandle()` (`src/project/tree.h`/`.cpp`). This mirrors novelWriter's handle
  scheme.

## Storage Format

The on-disk project format is still evolving; it will eventually be a zip of JSON files.
Until that lands, don't add dirty-flag tracking to save paths.
