#!/bin/bash

if ! command -v msgfmt &> /dev/null
then
	echo "Error: 'msgfmt' (gettext) is not installed."
	exit 1
fi

error_count=0

function notify() {
	[ -n "$CI" ] && while read l
	do
		file=$(cut -d ':' -f 1 <<< "$l" | cut -b 3-)
		line=$(cut -d ':' -f 2 <<< "$l")
		text=$(cut -d ':' -f 1-$2 --complement <<< "$l" | cut -b 2-)
		printf "::%s file=%s,line=%s::%s\n" "$3" "$file" "$line" "$text"
	done <<<"$1"
}

while IFS= read -r file
do
	output=$(msgfmt --check --output-file /dev/null "$file" 2>&1)

	output=$(grep -v "entries do not both end with '\\\\n'$" <<< "$output") # monolingual, so this error is invalid
	output=$(grep -v "^msgfmt: found [0-9]" <<< "$output") # the count will be wrong, so remove

	errors=$(grep -v ": warning: " <<< "$output")
	warnings=$(grep ": warning: " <<< "$output")

	if [ -z "$errors" ]
	then
		echo "[OK] $file"
	else
		echo "[FAIL] $file"
		((error_count++))
	fi


	if [ -n "$output" ]
	then
		[ -n "$errors" ] && notify "$errors" 2 error
		[ -n "$warnings" ] && notify "$warnings" 3 warning
		echo "$output" | sed 's/^/  /' # indent
	fi
done < <(find . -type f -name "*.po")

function duplicates() {
	filename="$1"; shift

	delim=';'

	declare -A strings

	maxkeylen=0

	for folder in "$@"
	do
		while IFS= read -r file
		do
			while read -r key
			do
				if [ -z "${strings[$key]}" ]
				then
					strings[$key]="${file}"
				else
					len=${#key}
					(( len > maxkeylen )) && maxkeylen=$len
					strings[$key]="${strings[$key]}${delim}${file}"
				fi
			done < <(sed -E '/^msgctxt/{N;s/^msgctxt "(.+)"\nmsgid "(.+)"/msgid "\1.\2"/}' "$file" | sed -En 's/^msgid "(.+)"$/\1/p')
		done < <(find "$folder" -type f -name "$filename")
	done

	dupes=0
	lines=()
	((maxkeylen++))

	for key in "${!strings[@]}"
	do
		value="${strings[$key]}"
		delims="${value//[^$delim]}"
		count=$((${#delims} + 1))
		if [ $count -gt 1 ]
		then
			IFS="$delim" read -ra files <<< "$value"
			for i in "${!files[@]}"
			do
				files[$i]="'${files[$i]%$filename}'"
			done
			lines+=( "$(printf "%-${maxkeylen}s" "$key:") ${files[*]}")
			(( count > dupes )) && dupes=$count
		fi
	done

	if [ "${#lines[@]}" -gt 0 ]
	then
		echo "Duplicate keys in [ $@ ]"
		for line in "${lines[@]}"
		do
			echo -e "${line}"
		done
	fi

	return $dupes
}
echo

duplicates en_US.po games engine
duplicates en_US.po engine
count=$?
((error_count+=$count))

if [ "$error_count" -gt 0 ]
then
	echo ""
	echo "Validation failed for $error_count file(s)."
	exit 1
fi

echo "All files valid."
