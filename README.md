# Collett

A rich text editor for novels written in Qt/C++.

This is another attempt at creating this app, and the third project with the same name.

The project is an alternative implementation of [novelWriter](https://github.com/saga-soft/novelWriter).

This is an experimental repo for the time being. It is not a usable application yet.

## Building

### Dependencies

On Debian or Ubuntu:

```bash
sudo apt install cmake ninja-build \
    qt6-base-dev qt6-svg-dev qt6-tools-dev qt6-l10n-tools \
    libnuspell-dev
```

Spell checking needs at least one Hunspell-compatible dictionary, for instance
`hunspell-en-gb` or `hunspell-nb`. Coverage reports need `gcovr`, which can be
installed with `pip install gcovr`.

### Build

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/collett
```

Use `-DCMAKE_BUILD_TYPE=Debug` for a debug build.

### Tests

The tests are built by default as part of the normal build. Run them with:

```bash
cd build && ctest --output-on-failure
```

A single test executable can also be run directly, for instance
`./build/tests/tst_spellcheck`. Pass `-DCOLLETT_BUILD_TESTS=OFF` to CMake to
skip building the tests.

### Coverage

The coverage build instruments the core library and adds a `coverage` target
that runs the tests and writes a report to `build_cov/coverage/index.html`:

```bash
cmake -S . -B build_cov -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCOLLETT_COVERAGE=ON
cmake --build build_cov --target coverage
```
