"""
Simple pretty-printing function

Copyright 2026 Marcus Minhorst

SPDX-License-Identifier: 0BSD
"""


def pretty(data, stack=None):
    """Pretty prints data in a yaml-like output"""

    if not stack:
        stack = []

    def line(data, end=""):
        print(f"{''.join(stack)}{str(data)}{end}")

    def entry(value, end):
        nonlocal stack
        pretty(value, stack + [end])
        stack = ["\t"] * len(stack)

    if isinstance(data, list):
        for value in data:
            entry(value, "- ")
    elif isinstance(data, dict):
        for key, value in data.items():
            line(key, ":")
            entry(value, "\t")
    else:
        line(data)
