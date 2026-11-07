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

def read_file(file):
	"""read a file, preprocess it in a way that the c preprocessor might"""

	defs={}
	lines=[]

	while line := file.readline():
		if line[0] != "#":
			lines += [line]
		else:
			split = line.split()
			if split[0] == "#define":
				defs[split[1]] = split[2]

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
	parser.add_argument("zip_file")
	parser.add_argument("directories", nargs="+")

	args = parser.parse_args()

	zip_path = args.zip_file
	dirs = args.directories

	defs = {}
	if args.m and os.path.isfile(args.m):
		with open(args.m, "r") as file:
			defs, _ = read_file(file)

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

					with open(filepath, "rb") as f:
						zipf.writestr(info, f.read())

	if not args.q:
		print("Done")

if __name__ == '__main__':
	main()
