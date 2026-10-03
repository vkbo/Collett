[![Build and Test](https://github.com/saga-soft/Collett/actions/workflows/build-and-test.yml/badge.svg)](https://github.com/saga-soft/Collett/actions/workflows/build-and-test.yml)
[![CodeQL](https://github.com/saga-soft/Collett/actions/workflows/github-code-scanning/codeql/badge.svg)](https://github.com/saga-soft/Collett/actions/workflows/github-code-scanning/codeql)

# Collett (Experimental)

A rich text editor for novels written in Qt/C++.

The project is an alternative implementation of [novelWriter](https://github.com/saga-soft/novelWriter). It is intended
to be feature-compatible with novelWriter, but will have its own project file format and a different approach to
tracking notes and meta data.

Note: This is an experimental repo for the time being. It is not a usable application yet.

The user interface is built with Qt Quick (QML).

## Building

### Dependencies

On Debian or Ubuntu:

```bash
sudo apt install cmake qt6-base-dev qt6-declarative-dev qt6-declarative-dev-tools qt6-svg-dev
```

Formatting checks need `clang-format`, which can be installed with:

```bash
sudo apt install clang-format
```

### Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/collett
```

Use `-DCMAKE_BUILD_TYPE=Debug` for a debug build. Add `-j` to the build command to compile in parallel, for instance
`cmake --build build -j 8`.

### Formatting

Source files are formatted according to the `.clang-format` file in the repository root. Requires `clang-format` to
be installed. Run `./format.sh` to format all source files in place before making a pull request, or `./format.sh
--check` to only check for violations without changing anything, as done in CI.
