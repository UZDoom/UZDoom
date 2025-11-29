#!/usr/bin/env python3

"""
Usage: ./compile.py path_to_po_files path_to_output.csv
"""

import sys
import csv
import polib
from pathlib import Path

SOURCE_LANG = "en_US"

def dump_csv(destination, header, data):
	table = [ header ] + list(data.values())
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

def main(args):
	if len(args) != 3:
		return print(__doc__)

	po_path = Path(args[1])

	if not po_path.is_dir():
		return print(__doc__)

	po_files = {}
	for f in po_path.iterdir():
		if f.is_file() and str(f).endswith(".po"):
			po_files[f.parts[-1][0:-3]] = f

	if SOURCE_LANG not in po_files:
		po_path = str(po_path / f"${SOURCE_LANG}.po")
		return print(f"{po_path} not found")

	header = []
	strings = {}

	current=fill_dict(po_files[SOURCE_LANG])
	header += [ current["meta"]["id"], "Identifier", "Remarks", "Filter" ]
	for k in current["data"]:
		v = current["data"][k]
		strings[k] = [
			v["string"] if "string" in v else "",
			k,
			v["remarks"] if "remarks" in v else "",
			v["filter"] if "filter" in v else ""
		]
	del po_files[SOURCE_LANG]

	for k in po_files:
		current = fill_dict(po_files[k])
		header += [ current["meta"]["id"] ]
		for kk in strings:
			strings[kk] += [ current["data"][kk]["string"] if kk in current["data"] and "string" in current["data"][kk] else "" ]

	dump_csv(args[2], header, strings)

if __name__ == "__main__":
    main(sys.argv)
