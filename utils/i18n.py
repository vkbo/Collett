"""
Collett - Translation Utils
===========================

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
import re
import shutil
import subprocess
import sys
import xml.etree.ElementTree as ET

from pathlib import Path

from utils.common import ROOT_DIR, envValue, log

I18N_DIR = ROOT_DIR / "i18n"
SOURCE_DIRS = ["src", "qml"]
SOURCE_LANGUAGE = "en_GB"

# The base file holds all source text, for the translation platform, and is
# not built. The source language has no translation file.
BASE_FILE = "collett_base.ts"
SOURCE_FILE = f"collett_{SOURCE_LANGUAGE}.ts"

# A language code with a country or region code, like nb_NO or es_419
TS_NAME_RE = re.compile(r"^collett_(base|[a-z]{2,3}_[A-Z]{2}|[a-z]{2,3}_[0-9]{3})\.ts$")


def _findLUpdate() -> str | None:
    """Find lupdate in the Qt kit of the toolchain file, or on the path."""
    if toolchain := envValue("QT_TOOLCHAIN_FILE"):
        # The toolchain file is in <kit>/lib/cmake/Qt6
        lupdate = Path(toolchain).parents[3] / "bin" / "lupdate"
        if lupdate.is_file():
            return str(lupdate)
    for name in ["lupdate-qt6", "lupdate"]:
        if found := shutil.which(name):
            return found
    lupdate = Path("/usr/lib/qt6/bin/lupdate")
    return str(lupdate) if lupdate.is_file() else None


def _normaliseTsLocations(tsFile: Path) -> tuple[int, int]:
    """Strip volatile TS line locations and merge duplicate file locations."""
    tree = ET.parse(tsFile)
    root = tree.getroot()

    nLines = 0
    nMerged = 0

    for message in root.findall("./context/message"):
        seenFiles: set[str] = set()
        for location in list(message.findall("location")):
            if "line" in location.attrib:
                del location.attrib["line"]
                nLines += 1

            if (filename := location.attrib.get("filename", "")) in seenFiles:
                message.remove(location)
                nMerged += 1
            else:
                seenFiles.add(filename)

    if nLines > 0 or nMerged > 0:
        ET.indent(tree, space="    ")
        header = '<?xml version="1.0" encoding="utf-8"?>\n<!DOCTYPE TS>\n'
        xmlBody = ET.tostring(root, encoding="unicode", short_empty_elements=True)
        xmlBody = xmlBody.replace(" />", "/>")
        tsFile.write_text(f"{header}{xmlBody}\n", encoding="utf-8")

    return nLines, nMerged


def updateTranslationSources(args: argparse.Namespace) -> None:
    """Create or update the translation files from the source code.

    The files must be in the i18n folder, and named collett_<lang>_<country>.ts,
    or collett_base.ts for the base file. Missing files are created. The
    source language is skipped, as it has no translation.
    """
    log("")
    log("[b]Update Translation Files[e]")
    log("[b]========================[e]")
    log("")

    if not (lupdate := _findLUpdate()):
        log("[cr]Error:[e] Could not find lupdate. Set QT_TOOLCHAIN_FILE, or install the Qt linguist tools.")
        sys.exit(1)
    log(f"[b]Using:[e] {lupdate}")
    log("")

    translations = []
    for item in [Path(f).absolute() for f in args.files]:
        if item.parent != I18N_DIR or item.name == SOURCE_FILE or not TS_NAME_RE.match(item.name):
            log(f"[cy]Skipped:[e] {item}")
            continue

        if not item.exists():
            langCode = SOURCE_LANGUAGE if item.name == BASE_FILE else item.stem.removeprefix("collett_")
            item.write_text(
                '<?xml version="1.0" encoding="utf-8"?>\n'
                "<!DOCTYPE TS>\n"
                f'<TS version="2.1" language="{langCode}" sourcelanguage="{SOURCE_LANGUAGE}"/>\n',
                encoding="utf-8",
            )
            log(f"[cg]Created:[e] {item.relative_to(ROOT_DIR)}")
        translations.append(item)

    log("")
    log("[b]Updating Translation Files:[e]")
    log("")

    for item in translations:
        cmd = [lupdate, *SOURCE_DIRS, "-extensions", "cpp,h,qml", "-no-obsolete", "-locations", "relative"]
        cmd.extend(["-ts", str(item)])
        if subprocess.call(cmd, cwd=ROOT_DIR) != 0:
            log(f"[cr]Failed:[e] {item.relative_to(ROOT_DIR)}")
            sys.exit(1)

    log("")
    log("[b]Normalising Locations:[e]")
    log("")

    for item in translations:
        nLines, nMerged = _normaliseTsLocations(item)
        if nLines > 0 or nMerged > 0:
            log(f"[cg]Updated:[e] {item.relative_to(ROOT_DIR)} ({nLines} line refs, {nMerged} merged)")
        else:
            log(f"[cy]No Change:[e] {item.relative_to(ROOT_DIR)}")

    log("")
