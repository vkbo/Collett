"""
Collett - Build Utils
=====================

This file is a part of Collett
Copyright (C) 2026 Veronica Berglyd Olsen

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.
"""  # noqa

from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import sys

from utils.common import ROOT_DIR, log, readEnvFile

BUILD_DIR = ROOT_DIR / "build"
BUILD_OPTIONS = ["debug", "release", "tests", "clean"]


def _run(cmd: list[str]) -> None:
    """Run a command from the project root, and exit if it fails."""
    log(f"[b]Running:[e] {' '.join(cmd)}")
    if subprocess.call(cmd, cwd=ROOT_DIR) != 0:
        log(f"[cr]Failed:[e] {cmd[0]}")
        sys.exit(1)
    log("")


def _cacheValue(key: str) -> str | None:
    """Look up a value in the build folder's CMake cache."""
    cache = BUILD_DIR / "CMakeCache.txt"
    if cache.is_file():
        for line in cache.read_text(encoding="utf-8").splitlines():
            name, _, value = line.partition("=")
            if name.partition(":")[0] == key:
                return value
    return None


def buildCollett(options: list[str]) -> None:
    """Configure and build Collett.

    The options are words from BUILD_OPTIONS. "clean" deletes the build
    folder first. "debug" or "release" sets the build type, which otherwise
    stays as it is, or is Debug for a new build folder. "tests" builds the
    unit tests, which are otherwise left out. The Qt kit is taken from the
    toolchain file set in QT_TOOLCHAIN_FILE in the environment or the .env
    file. Without it, CMake finds Qt itself.
    """
    if "debug" in options and "release" in options:
        log("[cr]Error:[e] Cannot build both 'debug' and 'release'.")
        sys.exit(1)

    log("")
    log("[b]Build Collett[e]")
    log("[b]=============[e]")
    log("")

    if "clean" in options and BUILD_DIR.is_dir():
        log(f"[b]Deleting:[e] {BUILD_DIR}")
        shutil.rmtree(BUILD_DIR)
        log("")

    cmd = ["cmake", "-S", ".", "-B", str(BUILD_DIR), "-G", "Unix Makefiles"]
    if "release" in options:
        cmd.append("-DCMAKE_BUILD_TYPE=Release")
    elif "debug" in options or _cacheValue("CMAKE_BUILD_TYPE") is None:
        cmd.append("-DCMAKE_BUILD_TYPE=Debug")
    cmd.append(f"-DCOLLETT_BUILD_TESTS={'ON' if 'tests' in options else 'OFF'}")
    if toolchain := os.environ.get("QT_TOOLCHAIN_FILE") or readEnvFile().get("QT_TOOLCHAIN_FILE"):
        cmd.append(f"-DCMAKE_TOOLCHAIN_FILE={toolchain}")
    _run(cmd)

    _run(["cmake", "--build", str(BUILD_DIR), "--parallel", str(os.cpu_count() or 1)])
    log(f"[cg]Build Done:[e] {_cacheValue('CMAKE_BUILD_TYPE')}")
    log("")


def build(args: argparse.Namespace) -> None:
    """Build entry point."""
    buildCollett(args.options)


def test(args: argparse.Namespace) -> None:
    """Run the unit tests, building them first if asked to."""
    if args.build:
        buildCollett(["tests"])

    log("")
    log("[b]Test Collett[e]")
    log("[b]============[e]")
    log("")

    if _cacheValue("COLLETT_BUILD_TESTS") != "ON":
        log("[cr]Error:[e] The tests are not built. Run 'build tests' first, or add '--build'.")
        sys.exit(1)

    _run(["ctest", "--test-dir", str(BUILD_DIR), "--output-on-failure"])
    log("[cg]Tests Done[e]")
    log("")
