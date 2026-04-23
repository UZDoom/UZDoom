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

	filename="${file##*/}"
	[[ "$filename" == *?.* ]] && extension="${filename##*.}" || extension=""
	extension="${extension,,}"
	data=$(cat "$file")

	((tested++))
	failed=

	[[ -n "$verbose" ]] && printf "Testing EOF newline: %s\n" "$file"
	end=$(tail -c 2 "$file" | od -An -tx1)
	if [[ "$end" == *" 0a 0a"* ]] || [[ "$end" != *" 0a"* ]]; then
		failed=1
		printf "EOF newline: %s\n" "$file" >&2
		# this will get automagically handled later lol
	fi

	[[ -n "$verbose" ]] && printf "Testing CRLF: %s\n" "$file"
	line_endings=$(file "$file")
	if grep -q $'\r' <<<"$data"; then
		failed=1
		printf "CRLF: %s\n" "$file" >&2
		[[ -n "$dry" ]] || data="${data//$'\r'/}"
	fi

	[[ -n "$verbose" ]] && printf "Trailing spaces: %s\n" "$file"
	if grep -q "[[:blank:]]$" <<<"$data"; then
		failed=1
		printf "Trailing spaces: %s\n" "$file" >&2
		[[ -n "$dry" ]] || data=$(sed 's/[[:blank:]]*$//' <<<"$data")
	fi

	tabs=
	indent=
	case "$extension" in
		c|cpp|h|mm|hpp|zs|fp|re|lemon|y|cmake|xml)
			tabs=true
		;;
		*)
			case "$filename" in
				CMakeLists.txt|*defs.*|*def.*)
					tabs=true
				;;
			esac
		;;
	esac
	if [ -z "$tabs" ]; then
		[[ -n "$verbose" ]] && printf "Skipping leading spaces: %s\n" "$file"
	else
		[[ -n "$verbose" ]] && printf "Leading spaces: %s\n" "$file"
		if grep -q "^    " <<<"$data"; then
			failed=1
			printf "Leading spaces: %s\n" "$file" >&2
			indent=1
		fi
	fi

	[[ -n "$verbose" ]] && printf "Inconsistent indentation: %s\n" "$file"
	if grep -q "^    " <<<"$data" && grep -q "^	" <<<"$data"; then
		failed=1
		printf "Inconsistent indentation: %s\n" "$file" >&2
		indent=1
	fi

	if [[ -n "$indent" ]]; then
		space=$(grep -P "^ +" -o <<<"$data")
		tabs=$(grep -P "^	" -o <<<"$data" | wc -l)
		width=4
		if [[ -n "$space" ]] && [[ $(wc -l <<<"$space") > $tabs ]]; then
			# guess intended indentation by counting the most common difference between indentation levels
			width=$(awk 'NR%2==1 { last_len=length($0); next }
			{
			  diff = length($0) - last_len;
			  diff = (diff < 0 ? -diff : diff);
			  if (diff != 0) { print diff }
			}' <<<"$space" | sort -n | uniq -c | sort -rn | head -n 1 | awk '{print $2}')
			[[ $width != 2 ]] && [[ $width != 4 ]] && [[ $width != 8 ]] && width=4
		fi

		[[ -n "$dry" ]] || data=$(printf "%s" "$data" | unexpand -t $width --first-only)
	fi

	if [[ -n "$failed" ]]; then
		((formatted++))
		[[ -n "$dry" ]] || { printf "%s\n" "$data" > "$file" ; }
		[[ -n "$failfast" ]] && exit 1
	fi
done < <(git ls-files -z)

printf "Checked: %d\nFormatted: %d\n" "$tested" "$formatted"

if [[ -n "$check" ]] && [[ $formatted != 0 ]]; then
	exit 1
fi
