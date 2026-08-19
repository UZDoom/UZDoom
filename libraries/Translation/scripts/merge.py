#!/usr/bin/env python3

"""
Merges 2 keys into 1, using a macro
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


def joinstr(a, b, d=None):
    """yep"""
    if d is not None:
        aa = a.split(d)
        bb = b.split(d)
        if len(aa) > 1 or len(bb) > 1:
            i = 0
            while i < min(len(aa), len(bb)) and aa[i] == bb[i]:
                i += 1
            return "%s%s{%s|%s}" % (
                d.join(aa[0:i]),
                d if i != 0 else '',
                d.join(aa[i:]),
                d.join(bb[i:])
            )
    i = 0
    while i < min(len(a), len(b)) and a[i] == b[i]:
        i += 1
    return f"{a[0:i]}{{{a[i:]}|{b[i:]}}}"


def mergekey(pofile, a, b):
    """yep"""
    key = joinstr(a, b)

    a_entry = pofile.find(a, by="msgid")
    b_entry = pofile.find(b, by="msgid")

    if not a_entry:
        a_entry = b_entry
    if not b_entry:
        b_entry = a_entry

    entry = ""
    if not a_entry:
        pass
    elif a_entry.msgstr != b_entry.msgstr:
        if a_entry.msgctxt != b_entry.msgctxt:
            return "msgctxt cannot differ"
        entry = joinstr(a_entry.msgstr, b_entry.msgstr, ' ')
    else:
        entry = a_entry.msgstr

    if a_entry:
        pofile.remove(a_entry)
        if a_entry != b_entry:
            pofile.remove(b_entry)
    else:
        a_entry = polib.POEntry()

    a_entry.msgstr = entry
    a_entry.msgid = key

    pofile.append(a_entry)

    print(f"{key}: {entry}")

    return None


def main(args):
    """yep"""

    source = Path(args.source)
    if not source.is_dir():
        print(f"{source} does not exist")
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

    keyset = set(args.key)
    if "" in keyset:
        print("keys cannot contain an empty value")
        sys.exit(1)
    if len(keyset) != len(args.key):
        print("keys cannot contain duplicates")
        sys.exit(1)

    if args.method == "merge":
        if len(args.key) != 2:
            print("exactly 2 keys must be provided")
            sys.exit(1)

        def action(f):
            return mergekey(f, args.key[0], args.key[1])
    else:
        print(f"unknown method '{args.method}'")
        sys.exit(1)

    for f in source.iterdir():
        pofile = polib.pofile(f, check_for_duplicates=True)
        action(pofile)
        if translator:
            set_meta("Last-Translator", translator, pofile)
        set_meta("X-Generator", SCRIPT_ID, pofile)
        set_meta("PO-Revision-Date", TIMESTAMP, pofile)

        pofile.save(f)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        prog=Path(__file__).name,
        description=__doc__)

    parser.add_argument(
        "--source", "-s",
        required=True,
        help='Source component')
    parser.add_argument(
        "--method", "-m",
        required=True,
        help='Merging method')
    parser.add_argument('key', nargs='+')

    main(parser.parse_args(sys.argv[1:]))
