#!/usr/bin/env python3

"""
Copyright 2025-2026 UZDoom Maintainers and Contributors

SPDX-License-Identifier: GPL-3.0-or-later

Usage: zipdir.py [-q] <zip file> <directory> ...

Given one or more directories, their contents are scanned recursively.
The base directory names are not stored in the zip file, but subdirectories recursed into are stored.

Options:
-q    Do not list files
"""

# TODO add: -u  Only update changed files

import argparse
import os
import sys
import zipfile

def main():
	parser = argparse.ArgumentParser()
	parser.add_argument("-q", action="store_true")
	parser.add_argument("zip_file")
	parser.add_argument("directories", nargs="+")

	args = parser.parse_args()

	zip_path = args.zip_file
	dirs = args.directories

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
					arcname = os.path.relpath(filepath, start=d_norm)
					if not args.q:
						print(f"  adding: {arcname}")
					zipf.write(filepath, arcname)
	if not args.q:
		print("Done")

if __name__ == '__main__':
	main()
