"""
Collett - Packaging Utils Main
==============================

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
import shutil

import utils.gutenberg
import utils.icons

from utils.common import ROOT_DIR, extractVersion, isStableVersion, log


def printVersion(args: argparse.Namespace) -> None:
    """Print the Collett version and exit."""
    print(extractVersion(beQuiet=True)[0], end=None)


def printChannel(args: argparse.Namespace) -> None:
    """Print 'stable' or 'pre' depending on the release channel, and exit."""
    print("stable" if isStableVersion() else "pre", end=None)


def buildSample(args: argparse.Namespace) -> None:
    """Build a sample project from a Project Gutenberg HTML file."""
    utils.gutenberg.main(args.source, args.target)


def cleanBuildDirs(args: argparse.Namespace) -> None:
    """Recursively delete the build folders."""
    log("")
    log("[b]Cleaning up build environment ...[e]")
    log("")

    folders = [
        ROOT_DIR / "build",
        ROOT_DIR / "dist",
    ]

    for folder in folders:
        if folder.is_dir():
            try:
                shutil.rmtree(folder)
                log(f"[cg]Deleted:[e] {folder}")
            except OSError:
                log(f"[cr]Failed:[e]  {folder}")
        else:
            log(f"[cy]Missing:[e] {folder}")

    log("")


def main() -> None:
    """Parse command line options and run the commands."""
    parser = argparse.ArgumentParser(
        usage="pkgutils.py [command] [--flags]",
        description="This tool provides build and developer commands for Collett.",
    )
    parsers = parser.add_subparsers()

    # Version
    cmdVersion = parsers.add_parser("version", help="Print the Collett version.")
    cmdVersion.set_defaults(func=printVersion)

    # Release Channel
    cmdChannel = parsers.add_parser("channel", help="Print 'stable' or 'pre' depending on the release channel.")
    cmdChannel.set_defaults(func=printChannel)

    # Additional Builds
    # =================

    # Build Icons
    cmdIcons = parsers.add_parser("icons", help="Build icon theme files from upstream sources.")
    cmdIcons.add_argument("--work-dir", help="Working directory.")
    cmdIcons.set_defaults(func=utils.icons.main)

    # Build Sample
    cmdBuildSample = parsers.add_parser("sample", help="Build a sample project from a Project Gutenberg HTML file.")
    cmdBuildSample.add_argument("source", help="Path to the HTML file.")
    cmdBuildSample.add_argument("target", help="Path to the project folder to create.")
    cmdBuildSample.set_defaults(func=buildSample)

    # Build Clean
    cmdBuildClean = parsers.add_parser("build-clean", help="Recursively delete all build folders.")
    cmdBuildClean.set_defaults(func=cleanBuildDirs)

    args = parser.parse_args()
    if hasattr(args, "func"):
        args.func(args)
    else:
        parser.print_help()
