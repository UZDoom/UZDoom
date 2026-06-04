#!/usr/bin/env python3

"""
Tests that everything is okay in the database
"""

import sys
from pathlib import Path

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

    if issues:
        pretty(issues)
        sys.exit(1)


if __name__ == "__main__":
    main()
