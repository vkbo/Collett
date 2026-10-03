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

from pathlib import Path

from utils.common import ROOT_DIR, envValue, log

BUILD_DIR = ROOT_DIR / "build"
COVERAGE_DIR = ROOT_DIR / "build_cov"
BUILD_OPTIONS = ["debug", "release", "tests", "coverage", "clean"]


def _run(cmd: list[str]) -> None:
    """Run a command from the project root, and exit if it fails."""
    log(f"[b]Running:[e] {' '.join(cmd)}")
    if subprocess.call(cmd, cwd=ROOT_DIR) != 0:
        log(f"[cr]Failed:[e] {cmd[0]}")
        sys.exit(1)
    log("")


def _cacheValue(key: str, buildDir: Path = BUILD_DIR) -> str | None:
    """Look up a value in a build folder's CMake cache."""
    cache = buildDir / "CMakeCache.txt"
    if cache.is_file():
        for line in cache.read_text(encoding="utf-8").splitlines():
            name, _, value = line.partition("=")
            if name.partition(":")[0] == key:
                return value
    return None


def buildCollett(options: list[str], buildDir: Path = BUILD_DIR) -> None:
    """Configure and build Collett.

    The options are words from BUILD_OPTIONS. "clean" deletes the build
    folder first. "debug" or "release" sets the build type, which otherwise
    stays as it is, or is Debug for a new build folder. "tests" builds the
    unit tests, which are otherwise left out. "coverage" instruments the code
    for coverage, and also builds the tests. The Qt kit is taken from the
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

    if "clean" in options and buildDir.is_dir():
        log(f"[b]Deleting:[e] {buildDir}")
        shutil.rmtree(buildDir)
        log("")

    cmd = ["cmake", "-S", ".", "-B", str(buildDir), "-G", "Unix Makefiles"]
    if "release" in options:
        cmd.append("-DCMAKE_BUILD_TYPE=Release")
    elif "debug" in options or _cacheValue("CMAKE_BUILD_TYPE", buildDir) is None:
        cmd.append("-DCMAKE_BUILD_TYPE=Debug")
    coverage = "coverage" in options
    cmd.append(f"-DCOLLETT_BUILD_TESTS={'ON' if coverage or 'tests' in options else 'OFF'}")
    cmd.append(f"-DCOLLETT_COVERAGE={'ON' if coverage else 'OFF'}")
    if coverage and (gcovr := ROOT_DIR / ".venv" / "bin" / "gcovr").is_file():
        # The system's gcovr may be too old for the report options
        cmd.append(f"-DGCOVR_EXECUTABLE={gcovr}")
    if toolchain := envValue("QT_TOOLCHAIN_FILE"):
        cmd.append(f"-DCMAKE_TOOLCHAIN_FILE={toolchain}")
    _run(cmd)

    _run(["cmake", "--build", str(buildDir), "--parallel", str(os.cpu_count() or 1)])
    log(f"[cg]Build Done:[e] {_cacheValue('CMAKE_BUILD_TYPE', buildDir)}")
    log("")


def build(args: argparse.Namespace) -> None:
    """Build entry point."""
    buildCollett(args.options)


def test(args: argparse.Namespace) -> None:
    """Run the unit tests, building them first if asked to.

    A build with coverage runs the tests through the coverage target, which
    also writes the coverage report. With the coverage flag, the tests are
    built with coverage in their own folder, so the main build is left as it
    is. The counts from earlier runs are cleared first, so the report only
    covers this run. The report folder is where the editor's coverage view
    looks for it.
    """
    buildDir = COVERAGE_DIR if args.coverage else BUILD_DIR
    coverage = args.coverage or _cacheValue("COLLETT_COVERAGE") == "ON"
    if args.build or args.coverage:
        buildCollett(["coverage"] if coverage else ["tests"], buildDir)

    log("")
    log("[b]Test Collett[e]")
    log("[b]============[e]")
    log("")

    if _cacheValue("COLLETT_BUILD_TESTS", buildDir) != "ON":
        log("[cr]Error:[e] The tests are not built. Run 'build tests' first, or add '--build'.")
        sys.exit(1)

    if coverage:
        for counts in buildDir.rglob("*.gcda"):
            counts.unlink()
        _run(["cmake", "--build", str(buildDir), "--target", "coverage"])
        log(f"[b]Coverage:[e] {buildDir / 'coverage' / 'index.html'}")
        log(f"[b]Total:[e] {_coverageTotal(buildDir)}")
    else:
        _run(["ctest", "--test-dir", str(buildDir), "--output-on-failure"])
    log("[cg]Tests Done[e]")
    log("")


def _coverageTotal(buildDir: Path) -> str:
    """Return the total line coverage from the text report."""
    report = buildDir / "coverage" / "coverage.txt"
    if report.is_file():
        for line in report.read_text(encoding="utf-8").splitlines():
            if line.startswith("TOTAL"):
                return line.split()[-1]
    return "Unknown"
