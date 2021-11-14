# Collett

A rich text editor for novels based on [novelWriter](https://github.com/vkbo/novelWriter) and
written with C++ and Qt6.2.

Currently, the project just exists to test how much work it would be to do this in C++ rather than
in Python like the other project. I have not yet decided whether to complete it or not. It is
currently not in a usable state.

## Qt6 Libraries

Either build Qt6 from source, or download the source packages from the
[Qt Open Source](https://www.qt.io/download-open-source) page.

For the Collett build to find the Qt6 libraries, either specify the path via `-DCMAKE_PREFIX_PATH`
to the `cmake` command, or make a symlink from the binary folder to `lib/qt6` inside the source.
