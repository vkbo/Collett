#!/usr/bin/env python3
"""
Collett - Gutenberg Sample Project
==================================

Builds a sample project from a Project Gutenberg HTML novel, laid out with
"Letter" and "Chapter" headings in "chapter" divs, like the HTML edition of
Frankenstein (#84). The title page becomes a page document, each letter and
chapter becomes a chapter document with the heading as its title and no
text, and the text of each becomes a single scene with no title.

Usage:
    utils/gutenberg.py <book.html> <output folder>

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
"""

import json
import random
import re
import shutil
import sys

from datetime import datetime
from html.parser import HTMLParser
from pathlib import Path

INLINE = {"i": "i", "em": "i", "b": "b", "strong": "b"}
WORD_SEPARATORS = re.compile(r"[\s–—]+")


class BookParser(HTMLParser):
    """Collect the title page and the chapters as lists of blocks.

    A block is a (format, fragments) tuple, and a fragment is a
    (flags, text) tuple, matching the document format.
    """

    def __init__(self):
        super().__init__(convert_charrefs=True)
        self.title = ""
        self.subtitle = ""
        self.author = ""
        self.chapters = []
        self._tag = None
        self._fmt = ""
        self._frags = []
        self._inline = []
        self._chapter = None
        self._done = False

    def handle_starttag(self, tag, attrs):
        if self._done:
            return
        cls = dict(attrs).get("class", "") or ""
        if tag in INLINE:
            self._inline.append(INLINE[tag])
        elif tag == "br" and self._tag:
            self._frags.append(("".join(self._inline), "\n"))
        elif tag in ("h1", "h2", "h3", "p"):
            self._tag = tag
            self._frags = []
            self._fmt = {
                "right": "p:at",
                "poem": "p:al:in1",
            }.get(cls, "p:al")

    def handle_endtag(self, tag):
        if self._done:
            return
        if tag in INLINE and self._inline:
            self._inline.pop()
        elif tag == self._tag:
            self._tag = None
            text = "".join(t for _, t in self._frags)
            text = re.sub(r"[ \t\r]*\n[ \t\r]*", "\n", text).strip()
            if not text:
                return
            if tag == "h1" and not self.title:
                self.title = text
            elif tag == "h3" and not self.chapters and not self.subtitle:
                self.subtitle = text
            elif tag == "h2" and re.match(r"^(Letter|Chapter) \d+$", text):
                self._chapter = (text, [])
                self.chapters.append(self._chapter)
            elif tag == "h2" and text.startswith("by ") and not self.chapters:
                self.author = text
            elif tag == "h2":
                self._chapter = None
            elif tag == "p" and self._chapter:
                self._chapter[1].append((self._fmt, self._clean(self._frags)))

    def handle_data(self, data):
        if "*** END OF THE PROJECT GUTENBERG" in data:
            self._done = True
            self._chapter = None
        if self._tag and not self._done:
            self._frags.append(("".join(self._inline), re.sub(r"\s+", " ", data)))

    @staticmethod
    def _clean(frags):
        """Merge neighbouring fragments with the same format, and trim the
        whitespace at the start and end of the paragraph and around breaks.
        """
        merged = []
        for flags, text in frags:
            if merged and merged[-1][0] == flags:
                merged[-1] = (flags, merged[-1][1] + text)
            else:
                merged.append((flags, text))
        cleaned = []
        for i, (flags, text) in enumerate(merged):
            text = re.sub(r" *\n *", "\n", text)
            if i == 0:
                text = text.lstrip()
            if i == len(merged) - 1:
                text = text.rstrip()
            if text:
                cleaned.append((flags, text))
        return cleaned


def handle():
    return format(random.getrandbits(52), "013x")


def counts(blocks):
    words = 0
    chars = 0
    for _, frags in blocks:
        text = "".join(t for _, t in frags)
        words += len([w for w in WORD_SEPARATORS.split(text) if w])
        chars += len(text.rstrip())
    return words, chars


def write_document(path, blocks, stamp):
    content = []
    for fmt, frags in blocks:
        texts = [f"t{':' + ':'.join(flags) if flags else ''}|{text}" for flags, text in frags]
        if len(texts) == 1:
            content.append({"u:fmt": fmt, "u:txt": texts[0]})
        else:
            content.append({"u:fmt": fmt, "x:txt": texts})
    with open(path, "w", encoding="utf-8") as fh:
        fh.write("{\n")
        fh.write('  "c:format": "CollettDocument:1.0",\n')
        fh.write(f'  "c:meta": {{\n    "m:created": "{stamp}",\n    "m:updated": "{stamp}"\n  }},\n')
        fh.write('  "x:content": [\n')
        fh.write(",\n".join("    " + json.dumps(b, ensure_ascii=False) for b in content))
        fh.write("\n  ]\n}\n")


def main(source, target):
    parser = BookParser()
    parser.feed(Path(source).read_text(encoding="utf-8"))
    if not parser.chapters:
        sys.exit("No chapters found")

    target = Path(target)
    if target.exists():
        shutil.rmtree(target)
    (target / "project").mkdir(parents=True)
    (target / "content").mkdir()

    stamp = datetime.now().replace(microsecond=0).isoformat()
    items = []

    def add(level, title, blocks):
        h = handle()
        write_document(target / "content" / f"{h}.json", blocks, stamp)
        words, chars = counts(blocks)
        item = {"m:handle": h, "m:level": level, "m:order": len(items), "m:characters": chars, "m:words": words}
        if level in ("Partition", "Chapter"):
            item["m:expanded"] = True
        item["u:title"] = title
        items.append(item)
        return h

    # The title page is a page of text, not part of the chapter structure
    name = parser.title.rstrip(";:,. ") or "Novel"
    titlePage = [("h1:ac", [("", parser.title or name)])]
    if parser.subtitle:
        titlePage.append(("p:ac", [("", parser.subtitle)]))
    if parser.author:
        titlePage.append(("p:ac", [("", parser.author)]))
    add("Page", "", titlePage)

    # The heading of a letter or chapter is its title, so the chapter
    # document has no text of its own. The scenes have no titles.
    first = None
    for heading, blocks in parser.chapters:
        add("Chapter", heading, [])
        scene = add("Scene", "", blocks)
        first = first or scene

    structure = {
        "c:format": "CollettProjectStructure:1.0",
        "x:groups": [{"m:class": "Novel", "m:order": 0, "u:name": "Novel", "x:items": items}],
    }
    with open(target / "project" / "structure.json", "w", encoding="utf-8") as fh:
        json.dump(structure, fh, indent=2, ensure_ascii=False)
        fh.write("\n")

    project = {
        "c:format": "CollettProjectData:1.0",
        "c:meta": {"m:created": stamp, "m:updated": stamp, "m:version": "0.0.1-alpha1"},
        "c:project": {"u:name": name},
        "c:settings": {"m:lastEdited": first, "u:spellLanguage": None},
    }
    with open(target / "project" / "project.json", "w", encoding="utf-8") as fh:
        json.dump(project, fh, indent=2)
        fh.write("\n")
    (target / "CollettProject.collett").write_text("Collett 0.0.1-alpha1")

    total = sum(i["m:words"] for i in items)
    print(f"Wrote {len(items)} documents, {len(parser.chapters)} chapters, {total} words, to {target}")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        sys.exit(__doc__.split("This file")[0].strip())
    main(sys.argv[1], sys.argv[2])
