"""
Collett - Utils Common Functions
================================

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

import os
import sys

from pathlib import Path

ROOT_DIR = Path(__file__).parent.parent

# ANSI Colour Codes
ANSI_COLOURS = {
    "[e]": "\033[0m",  # Reset
    "[b]": "\033[1m",  # Bold
    "[cr]": "\033[91m",  # Bright red
    "[cg]": "\033[92m",  # Bright green
    "[cy]": "\033[93m",  # Bright yellow
}

NO_COLOR = bool(os.environ.get("NO_COLOR"))  # Non-empty value forces colour off
SUPPORTS_COLOUR = sys.stdout.isatty() and not NO_COLOR


def log(message: str | Path | Exception = "") -> None:
    """Print a message to the terminal, translating ANSI colour codes."""
    if isinstance(message, Exception):
        message = f"[cr]{message.__class__.__name__}:[e] {message!s}"
    else:
        message = str(message)

    if message:
        for code, ansi in ANSI_COLOURS.items():
            message = message.replace(code, ansi if SUPPORTS_COLOUR else "")

    print(message, flush=True)


def extractVersion(beQuiet: bool = False) -> tuple[str, str, str]:
    """Extract the Collett version number from the main header file."""

    def getValue(text: str) -> str:
        return text.split(maxsplit=2)[-1].strip().strip('"')

    numVers = "0"
    hexVers = "0x0"
    relDate = "Unknown"
    headerFile = ROOT_DIR / "src" / "collett.h"
    try:
        for line in headerFile.read_text(encoding="utf-8").splitlines():
            if line.startswith("#define COL_VERSION_STR"):
                numVers = getValue(line)
            elif line.startswith("#define COL_VERSION_NUM"):
                hexVers = getValue(line)
            elif line.startswith("#define COL_VERSION_DATE"):
                relDate = getValue(line)
    except Exception as exc:
        log(f"[cr]Could not read file:[e] {headerFile}")
        log(exc)

    if not beQuiet:
        log(f"Collett version: {numVers} ({hexVers}) at {relDate}")

    return numVers, hexVers, relDate


def isStableVersion() -> bool:
    """Return True if the version is a stable release."""
    _, hexVers, _ = extractVersion(beQuiet=True)
    return hexVers[-2] == "f"


def readEnvFile() -> dict[str, str]:
    """Read a simple KEY=VALUE .env file from the project root into a dict."""
    envFile = ROOT_DIR / ".env"
    values = {}
    if envFile.is_file():
        for line in envFile.read_text(encoding="utf-8").splitlines():
            line = line.strip()
            if not line or line.startswith("#") or "=" not in line:
                continue
            key, _, value = line.partition("=")
            values[key.strip()] = value.strip()
    return values
