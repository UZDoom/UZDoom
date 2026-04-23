#!/usr/bin/env bash

usage() {
	echo "Usage: $0 [-c] [-d] [-f] [-v]"
	echo
	echo "-c: check"
	echo "-d: dry run"
	echo "-f: fail fast"
	echo "-v: verbose"
	exit 1
}

while getopts "cdfvh" opt; do
	case "$opt" in
		c) check=true ;;
		d) dry=true ;;
		f) failfast=true ;;
		v) verbose=true ;;
		*) usage ;;
	esac
done

cd "$(git rev-parse --show-toplevel)" || exit

tested=0
formatted=0

while IFS= read -r -d '' file; do
	grep -qI . "$file" || continue # skip binary files

	# skip specific paths / files
	[[ "$file" == bin/* ]] && continue
	[[ "$file" == libraries/* ]] && continue
	[[ "$file" == tools/re2c/* ]] && continue
	[[ "$file" == tools/lemon/* ]] && continue
	[[ "$file" == */thirdparty/* ]] && continue

	((tested++))
	failed=

	[[ -n "$verbose" ]] && printf "Testing EOF newline: %s\n" "$file"
	end=$(tail -c 2 "$file" | od -An -tx1)
	if [[ "$end" == *" 0a 0a"* ]] || [[ "$end" != *" 0a"* ]]; then
		failed=1
		printf "EOF newline: %s\n" "$file" >&2
		[[ -n "$dry" ]] || { printf "%s\n" "$(cat "$file")" > "$file" ; }
	fi

	if [[ -n "$failed" ]]; then
		((formatted++))
		[[ -n "$failfast" ]] && exit 1
	fi
done < <(git ls-files -z)

printf "Checked: %d\nFormatted: %d\n" "$tested" "$formatted"

if [[ -n "$check" ]] && [[ $formatted != 0 ]]; then
	exit 1
fi
