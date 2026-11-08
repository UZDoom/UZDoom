#!/usr/bin/env python3

"""
Copyright 2025-2026 UZDoom Maintainers and Contributors

SPDX-License-Identifier: GPL-3.0-or-later

</war>
"""

import argparse
import os
import sys
import zipfile
import pathlib
from datetime import datetime, timezone

def read_file(file, gdefs):
	"""read a file, preprocess it in a way that the c preprocessor might"""

	defs = dict(gdefs)
	lines = []
	nesting = []

	while line := file.readline():
		if line.startswith("#"): # intentionally skip leading white-space
			split = line.strip().split()
			directive = split[0]

			if directive in ("#ifdef", "#ifndef"):
				condition = split[1] in defs
				if directive == "#ifndef":
					condition = not condition
				parent_active = all(state["active"] for state in nesting) if nesting else True
				nesting.append({
					"active": parent_active and condition,
					"branch_taken": condition,
					"else_seen": False
				})
				continue

			elif directive == "#else":
				if nesting:
					current = nesting[-1]
					if not current["else_seen"]:
						parent_active = all(state["active"] for state in nesting[:-1]) if len(nesting) > 1 else True
						current["active"] = parent_active and not current["branch_taken"]
						current["else_seen"] = True
				continue

			elif directive == "#endif":
				if nesting:
					nesting.pop()
				continue

			elif directive == "#undef":
				if not nesting or all(state["active"] for state in nesting):
					defs.pop(split[1], None)
				continue

			elif directive == "#define":
				if not nesting or all(state["active"] for state in nesting):
					defs[split[1]] = " ".join(split[2:]) if len(split) > 2 else True
					lines.append(line)
				continue

		if nesting and not all(state["active"] for state in nesting):
			continue

		lines.append(line)

	return defs, "".join(lines)

def main():
	"""
	Given one or more directories, their contents are scanned recursively.
	The base directory names are not stored in the zip file, but subdirectories recursed into are stored.
	"""

	parser = argparse.ArgumentParser(formatter_class=argparse.RawDescriptionHelpFormatter, description=main.__doc__, epilog=__doc__)
	parser.add_argument("-q", action="store_true", help="Do not list files")
	parser.add_argument("-d", action="store_true", help="Normalize date")
	parser.add_argument("-m", type=pathlib.Path, help="Macro file")
	parser.add_argument("-f", type=ascii, help="Extra flags")
	parser.add_argument("-p", type=ascii, help="Filetypes to preprocess")
	parser.add_argument("zip_file")
	parser.add_argument("directories", nargs="+")

	args = parser.parse_args()

	zip_path = args.zip_file
	dirs = args.directories

	defs = {}
	if args.f:
		for f in args.f[1:-1].split():
			defs[f] = True
	if args.m and os.path.isfile(args.m):
		with open(args.m, "r") as file:
			defs |= read_file(file, defs)[0]
	extensions = tuple([
		p if p.startswith(".") else f".{p}" for p in args.p[1:-1].lower().split()
	] if args.p else [])

	time=None
	if args.d:
		try:
			time = datetime.fromisoformat(defs["GIT_TIME"][1:-1])
		except Exception as e:
			print(e)

	for d in dirs:
		if not os.path.isdir(d):
			print(f"Error: `{d}` is not a valid directory.", file=sys.stderr)
			sys.exit(1)

	if not args.q:
		print(f"Updating: {zip_path}")

	with zipfile.ZipFile(zip_path, 'w', zipfile.ZIP_DEFLATED) as zipf:
		for d in sorted(dirs):
			d_norm = os.path.normpath(d)
			for root, subdirs, files in os.walk(d_norm):
				subdirs.sort()
				files.sort()
				for file in files:
					filepath = os.path.join(root, file)
					arcname = os.path.relpath(filepath, start=d_norm).replace("\\", "/")
					if not args.q:
						print(f"  adding: {arcname}")

					info = zipfile.ZipInfo(arcname)
					if time:
						info.date_time = time.timetuple()[:6]
					else:
						mtime = datetime.fromtimestamp(os.path.getmtime(filepath))
						info.date_time = mtime.timetuple()[:6]
					info.compress_type = zipfile.ZIP_DEFLATED
					info.create_system = 3
					info.external_attr = (0o644 & 0xFFFF) << 16

					preprocess = file.lower().endswith(extensions)
					with open(filepath, "r" if preprocess else "rb") as f:
						data = read_file(f, defs)[1] if preprocess else f.read()
						zipf.writestr(info, data)

	if not args.q:
		print("Done")

if __name__ == '__main__':
	main()
