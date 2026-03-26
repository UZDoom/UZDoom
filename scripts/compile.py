#!/usr/bin/env python3

"""
Builds language files
"""

import os
import sys
import csv
import json
import argparse
from pathlib import Path

import polib

RECIPES = {
    "ENGINE": ["engine/common", "engine/zdoom"],
    "GAMES": [
        "games/chex",
        "games/doom",
        "games/doom2",
        "games/heretic",
        "games/hexen",
        "games/hexen-deathkings",
        "games/plutonia",
        "games/strife",
        "games/tnt",
    ],
    "GAMES_CHEX3": ["games/filter/chex_quest_3"],
    "GAMES_HARMONY": ["games/filter/harmony"],
    "GAMES_HACX": ["games/filter/hacx"]
}

SOURCE_LANG = "en_US"

# auto-add once a certain portion of strings have been translated
THRESHOLD = 0.5

# add to table  even if THRESHOLD is not met
ENABLED = [
    "en_GB",  # eng enc ena enz eni ens enj enb enl ent enw
    "cs",
    "da",
    "de",
    "es",
    "es_MX",  # esm
    "eo",
    "fi",
    "fr",
    "hu",
    "it",
    "ja",  # jp
    "ko",
    "nl",
    "nb_NO",  # no
    "pl",
    "pt",  # ptg
    "pt_BR",  # pt
    "ro",
    "ru",
    "sr",
    "tr",
]

# Don't add to table even if THRESHOLD is met
# If adding a language here, please add a comment why, so it can be
# re-evaluated later
DISABLED = [
    "arz",  # no rtl support in uzdoom yet
    "he",  # no rtl support in uzdoom yet
]

KEEP_REMARKS = False

DEBUG = True
try:
    DEBUG = DEBUG or 'DEBUG_LANGUAGE' in os.environ
except OSError:
    pass


def dump_csv(destination, table):
    """Writes the matrix table to a CSV file at the specified destination."""

    with open(destination, mode='w', newline='', encoding='utf-8') as file:
        csv.writer(file).writerows(table)


def remap(s):
    """
    Maps proper chars to chars/sequences that uzdoom can understand.
    This is temporary, and eventually everything in here will be removed.
    """

    return s\
        .replace("™", "(TM)")\
        .replace("®", "(R)")\
        .replace("©", "(C)")\
        .replace("…", "...")\
        .replace("“", "\"")\
        .replace("”", "\"")\
        .replace("‘", "'")\
        .replace("’", "'")\
        .replace("‐", "-")\
        .replace("–", "-")\
        .replace("—", "-")\
        .replace("×", "x")\
        .replace(" ", " ")


def fill_dict(path):
    """Parses a .po file into a dictionary of translation data and metadata."""

    po = polib.pofile(path)

    meta = {}
    data = {}

    meta["id"] = po.metadata["Language"]
    meta["valid"] = True

    # for now uzdoom needs the top left cell to be "default"
    if meta["id"] == "en_US":
        meta["id"] = "default"

    for e in po:
        specific_id = e.msgid
        entry = {"id": e.msgid}

        if e.msgstr:
            entry["string"] = remap(e.msgstr)
        if e.tcomment:
            entry["remarks"] = e.tcomment
        if e.msgctxt:
            entry["filter"] = e.msgctxt
            specific_id = f"{specific_id}#{e.msgctxt}"

        if specific_id in data:
            if meta["valid"]:
                print(f"in: {path}")
            meta["valid"] = False
            print(f"redefining: {entry["msgid"]}")
            continue

        data[specific_id] = entry

    return {"data": data, "meta": meta}


def get_po_files(po_paths):
    """Validates directories and aggregates parsed
    data for all language files."""

    failed = False

    languages = {}
    po_files = []
    for po_path in po_paths:
        if not po_path.is_dir():
            failed = True
            print(f"{po_path} not a folder")
            continue

        _po_files = {}
        for f in po_path.iterdir():
            if f.is_file() and str(f).endswith(".po"):
                po_id = f.parts[-1][0:-3]
                _po_files[po_id] = fill_dict(f)
                if not _po_files[po_id]["meta"]["valid"]:
                    failed = True
                if po_id not in languages:
                    languages[po_id] = _po_files[po_id]["meta"]["id"]
                if languages[po_id] != _po_files[po_id]["meta"]["id"]:
                    failed = True
                    la = languages[po_id]
                    lb = _po_files[po_id]['meta']['id']
                    print(f"inconsistent language mapping {la} / {lb}")
                    break
        if failed:
            continue

        if SOURCE_LANG not in _po_files:
            failed = True
            po_path = str(po_path / f"{SOURCE_LANG}.po")
            print(f"{po_path} not found")
            continue

        po_files += [_po_files]

    if failed:
        return None

    languages = sorted([v for k, v in languages.items() if k != SOURCE_LANG])
    return {
        "files": po_files,
        "languages": languages
    }


def build_matrix(languages, po_files):
    """Aligns translations from different languages into a
    keyed matrix for CSV output."""

    matrix = {}

    for files in po_files:
        current = files[SOURCE_LANG]
        _matrix = {}

        for k in current["data"]:
            if k in matrix:
                print(f"Duplicate key {k}")
            v = current["data"][k]
            _matrix[k] = [
                v["string"] if "string" in v else "",
                v["id"],
                v["remarks"] if "remarks" in v and KEEP_REMARKS else "",
                v["filter"] if "filter" in v else ""
            ]

        files = {files[f]["meta"]["id"]: files[f]
                 for f in files if f != SOURCE_LANG}
        files = [files[f]["data"] if f in files else {} for f in languages]

        for k in _matrix:
            for f in files:
                tmp = [f[k]["string"] if (k in f and "string" in f[k]) else ""]
                _matrix[k] += tmp
            matrix[k] = _matrix[k]

    return matrix


def postprocess_matrix(languages, matrix):
    """make the matrix smaller"""

    if not matrix:
        return [languages, matrix]

    if DEBUG:
        notfound = [v for v in ENABLED + DISABLED if v not in languages]
        if notfound:
            print(notfound)

    langcount = len(matrix)
    tally = [0 for _ in languages]
    skip = 3  # default, remarks, filter

    for k in matrix:
        v = matrix[k]
        for i in range(skip, len(v)):
            if v[i]:
                tally[i - skip - 1] += 1
            # just use the fallback
            if v[i] == v[0]:
                v[i] = ""

    progress = {} if DEBUG else None
    for i, v in enumerate(tally):
        v /= langcount
        m = 1 * (languages[i] in ENABLED) - 1 * (languages[i] in DISABLED)
        tally[i] = (v + m) >= THRESHOLD
        if DEBUG:
            _a = f"{v:.2f}".lstrip('0')[:3]
            _b = (v >= THRESHOLD) * 1
            progress[languages[i]] = f"{_a}={_b}{m:+}"
    languages = [languages[i] for i in range(len(tally)) if tally[i]]
    tally = [True for i in range(skip + 1)] + tally
    if DEBUG:
        print(json.dumps(progress, separators=(',', ':')))

    for k in matrix:
        matrix[k] = [
            v for i, v in enumerate(
                matrix[k]) if tally[i]]

    return [languages, matrix]


def main(args):
    """loading, matrix building, CSV export"""

    root_dir = Path(__file__).parent.parent

    po_files = RECIPES[args.recipe] if args.recipe in RECIPES else None
    po_files = po_files and get_po_files([root_dir / f for f in po_files])

    if po_files is None:
        print(__doc__)
        print("Available recipes: " + " ".join(RECIPES.keys()))
        sys.exit(1)

    languages = po_files["languages"]
    po_files = po_files["files"]

    matrix = build_matrix(languages, po_files)
    [languages, matrix] = postprocess_matrix(languages, matrix)

    source_id = po_files[0][SOURCE_LANG]["meta"]["id"]
    header = [source_id, "Identifier", "Remarks", "Filter"] + languages

    table = [header] + [matrix[k] for k in sorted(matrix)]

    dump_csv(args.output, table)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        prog=Path(__file__).name,
        description=__doc__)

    parser.add_argument("--recipe", required=True)
    parser.add_argument("--output", required=True, type=Path)

    main(parser.parse_args(sys.argv[1:]))
