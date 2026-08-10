#!/usr/bin/env python3

"""
Tests that everything is okay in the database
"""

import os
import re
import sys
from pathlib import Path

from libs import polib
from pretty import pretty
from config import \
    RECIPES


issues = {}


def issue(message, **kwargs):
    """Tosses stuff into the issues dict"""
    for k, v in kwargs.items():
        if k not in issues:
            issues[k] = {}
        if v not in issues[k]:
            issues[k][v] = []
        issues[k][v] += [message]


def main():
    """yep"""

    for component in RECIPES["ALL"]:
        cpath = Path(component)
        if not cpath.is_dir():
            issue("not found", component=component)
            continue

        ppath = cpath / "template.pot"
        if not ppath.is_file():
            issue("not found", template=component)
            continue

        po = polib.pofile(ppath)
        pattern = re.compile('^[A-Z0-9_]+$')
        expected = set()
        for e in po:
            if not bool(pattern.match(e.msgid)):
                issue(e.msgid, invalid_name=ppath)
            expected.add(e.msgid)

        for file in os.listdir(cpath):
            if file.endswith(".po"):
                ppath = cpath / file
                po = polib.pofile(ppath)
                found = set()
                for e in po:
                    if not bool(pattern.match(e.msgid)):
                        issue(e.msgid, invalid_name=ppath)
                    expected.add(e.msgid)
                for e in found - expected:
                    issue(e, extra_key=ppath)

    if issues:
        pretty(issues)
        sys.exit(1)


if __name__ == "__main__":
    main()
