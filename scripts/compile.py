#!/usr/bin/env python3

"""
Usage: ./compile.py path_to_po_files path_to_output.csv
"""

import sys
import csv
import polib
from pathlib import Path

SOURCE_LANG = "en_US"

def dump_csv(destination, table):
	with open(destination, mode='w', newline='', encoding='utf-8') as file:
		csv.writer(file).writerows(table)

def fill_dict(path):
	po = polib.pofile(path)

	meta = {}
	data = {}

	meta["id"] = po.metadata["HeaderCode"] if "HeaderCode" in po.metadata else po.metadata["Language"]

	for entry in po:
		data[entry.msgid] = {}
		if entry.msgstr:
			data[entry.msgid]["string"] = entry.msgstr
		if entry.tcomment:
			data[entry.msgid]["remarks"] = entry.tcomment
		if entry.msgctxt:
			data[entry.msgid]["filter"] = entry.msgctxt

	return { "data": data, "meta": meta }

def get_po_files(po_paths):
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
				if not po_id in languages:
					languages[po_id] = _po_files[po_id]["meta"]["id"]
				if languages[po_id] != _po_files[po_id]["meta"]["id"]:
					failed = True
					print(f"inconsistent language mapping {languages[po_id]} / {_po_files[po_id]['meta']['id']}")
					break
		if failed:
			continue

		if SOURCE_LANG not in _po_files:
			failed = True
			po_path = str(po_path / f"{SOURCE_LANG}.po")
			print(f"{po_path} not found")
			continue

		po_files += [ _po_files ]

	if not failed:
		return {
			"files": po_files,
			"languages": sorted([ languages[k] for k in languages if k != SOURCE_LANG ])
		}

def build_matrix(languages, po_files):

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
				k,
				v["remarks"] if "remarks" in v else "",
				v["filter"] if "filter" in v else ""
			]

		files = { files[f]["meta"]["id"]: files[f] for f in files if f != SOURCE_LANG }
		files = [ files[f]["data"] if f in files else {} for f in languages ]

		for k in _matrix:
			for f in files:
				_matrix[k] += [ f[k]["string"] if ( k in f and "string" in f[k] ) else "" ]
			matrix[k] = _matrix[k]

	return matrix

def main(args):
	po_files = get_po_files([ Path(f) for f in args[1:-1] ]) if len(args) >= 3 else None

	if po_files is None:
		print(__doc__)
		exit(1)

	languages = po_files["languages"]
	po_files = po_files["files"]

	header = [ po_files[0][SOURCE_LANG]["meta"]["id"], "Identifier", "Remarks", "Filter" ] + languages
	matrix = build_matrix(languages, po_files)

	table = [ header ] + [ matrix[k] for k in sorted(matrix) ]

	dump_csv(args[-1], table)

if __name__ == "__main__":
    main(sys.argv)
