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

import utils.build
import utils.gutenberg
import utils.i18n
import utils.icons

from utils.common import extractVersion, isStableVersion


def printVersion(args: argparse.Namespace) -> None:
    """Print the Collett version and exit."""
    print(extractVersion(beQuiet=True)[0], end=None)


def printChannel(args: argparse.Namespace) -> None:
    """Print 'stable' or 'pre' depending on the release channel, and exit."""
    print("stable" if isStableVersion() else "pre", end=None)


def buildSample(args: argparse.Namespace) -> None:
    """Build a sample project from a Project Gutenberg HTML file."""
    utils.gutenberg.main(args.source, args.target)


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

    # Build and Test
    # ==============

    # Build
    cmdBuild = parsers.add_parser("build", help="Configure and build the app.")
    cmdBuild.add_argument(
        "options",
        nargs="*",
        choices=utils.build.BUILD_OPTIONS,
        help=(
            "Any of: clean (delete the build folder first), debug or release (the build type, "
            "which is otherwise kept, or Debug for a new build), tests (also build the tests), "
            "coverage (instrument for coverage, with tests)."
        ),
    )
    cmdBuild.set_defaults(func=utils.build.build)

    # Test
    cmdTest = parsers.add_parser("test", help="Run the unit tests.")
    cmdTest.add_argument("--build", action="store_true", help="Build the app and tests before running the tests.")
    cmdTest.add_argument(
        "--coverage",
        action="store_true",
        help="Build with coverage in build_cov and write a coverage report, also as lcov.info.",
    )
    cmdTest.set_defaults(func=utils.build.test)

    # Additional Builds
    # =================

    # Build Icons
    cmdIcons = parsers.add_parser("icons", help="Build icon theme files from upstream sources.")
    cmdIcons.add_argument("--work-dir", help="Working directory.")
    cmdIcons.set_defaults(func=utils.icons.main)

    # Update i18n Sources
    cmdUpdateTS = parsers.add_parser("qtlupdate", help="Create or update translation files for internationalisation.")
    cmdUpdateTS.add_argument("files", nargs="+", help="Translation files, like i18n/collett_nb_NO.ts.")
    cmdUpdateTS.set_defaults(func=utils.i18n.updateTranslationSources)

    # Build Sample
    cmdBuildSample = parsers.add_parser("sample", help="Build a sample project from a Project Gutenberg HTML file.")
    cmdBuildSample.add_argument("source", help="Path to the HTML file.")
    cmdBuildSample.add_argument("target", help="Path to the project folder to create.")
    cmdBuildSample.set_defaults(func=buildSample)

    args = parser.parse_args()
    if hasattr(args, "func"):
        args.func(args)
    else:
        parser.print_help()
