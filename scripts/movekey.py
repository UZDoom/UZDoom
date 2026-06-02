#!/usr/bin/env python3

"""
Moves key from one component to another
"""

import sys
import argparse
import subprocess
from pathlib import Path

from libs import polib
from config import \
    TIMESTAMP, \
    SCRIPT_ID


def set_meta(key, val, *pofiles):
    """Helper function. Sets metadata[key]=val in each pofile given"""

    for po in pofiles:
        po.metadata[key] = val


def movekey(source, destination, key, translator):
    """yep"""

    src_po = polib.pofile(source)

    value = src_po.find(key)

    if destination.is_file():
        dst_po = polib.pofile(destination, check_for_duplicates=True)
    else:
        dst_po = polib.POFile(check_for_duplicates=True)
        dst_po.metadata = src_po.metadata
        dst_po.header = src_po.header

    if value:
        if translator:
            set_meta("Last-Translator", translator, src_po, dst_po)
        set_meta("X-Generator", SCRIPT_ID, src_po, dst_po)
        set_meta("PO-Revision-Date", TIMESTAMP, src_po, dst_po)

        src_po.remove(value)
        dst_po.append(value)

    src_po.save(source)
    dst_po.save(destination)


def main(args):
    """yep"""

    source = Path(args.source)
    destination = Path(args.destination)

    if not source.is_dir():
        print(f"{source} does not exist")
        sys.exit(1)

    if not destination.is_dir():
        print(f"creating {destination}")
        try:
            destination.mkdir(parents=True, exist_ok=True)
        except OSError:
            print(f"failed to create {destination}")
            sys.exit(1)

    translator = None
    try:
        res = subprocess.run(["git", "config", "user.name"],
                             check=True, stdout=subprocess.PIPE)
        username = res.stdout.strip().decode()
        res = subprocess.run(["git", "config", "user.email"],
                             check=True, stdout=subprocess.PIPE)
        email = res.stdout.strip().decode()
        translator = f"{username} <{email}>"
    except subprocess.CalledProcessError:
        pass

    for key in args.key:
        for f in [f.relative_to(source) for f in source.iterdir()]:
            movekey(source / f, destination / f, key, translator)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        prog=Path(__file__).name,
        description=__doc__)

    parser.add_argument(
        "--source", "-s",
        required=True,
        help='Source component')
    parser.add_argument(
        "--destination", "-d",
        required=True,
        help='Destination component')
    parser.add_argument('key', nargs='*')

    main(parser.parse_args(sys.argv[1:]))
