#!/bin/bash

error_count=0
_phase=
phase() {
	if [[ -n "$_phase" ]]; then
		if [[ "$error_count" == 0 ]]; then
			echo "Okay"
		else
			echo "Checks failed in $error_count place(s)!" >&2
		fi
		echo
	fi
	printf "###\n"
	if [[ -n "$_phase" ]]; then
		printf "# END %s\n" "$_phase"
		if [[ "$error_count" != 0 ]]; then
			echo "###"
			exit 1
		elif [[ -n "$*" ]]; then
			printf "#\n"
		fi
	fi
	[[ -n "$*" ]] && printf "# BEGIN %s\n" "$*"
	printf "###\n\n"
	_phase="$*"
}

###
phase check for dependencies
###

commands=( # [n]:command [n+1]:source
	git      git
	msgfmt   gettext

	autopep8 pip,autopep8
	csvclean pip,csvkit
	pylint   pip,pylint
	vermin   pip,vermin
)
for ((i=0 ; i < ${#commands[@]} ; i+=2 )); do
	if command -v "${commands[i]}" &> /dev/null
	then
		echo "Found: '${commands[i]}'"
	else
		echo "Error: '${commands[i]}' (${commands[i+1]}) is not installed."
		((error_count++))
	fi
done
echo

###
phase validate python scripts
###

pyfiles=( scripts/compile.py )
pyversion=3.6

for pyfile in "${pyfiles[@]}"; do
	autopep8 --exit-code -aavi "${pyfile}" || ((error_count++))
	vermin --no-parse-comments --pessimistic --violations \
	--target="${pyversion}-" "${pyfile}" || ((error_count++))
	pylint --py-version="${pyversion}" "${pyfile}" || ((error_count++))
done

###
phase check templates
###

dirty=$(git status --porcelain)
bash ./scripts/mktemplate.sh || ((error_count++))
[[ -z "$dirty" ]] && { [[ -z "$(git status --porcelain)" ]] || ((error_count++)) ; }

###
phase test compile
###

scripts/compile.py --recipe ALL --output out.csv || ((error_count++))
t1=$(mktemp)
csvclean -a out.csv >$t1 || ((error_count++))
diff out.csv $t1 --brief
rm $t1
echo

###
phase validate po files
###

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
echo

###
phase check for duplicates
###

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

duplicates en_US.po games engine
duplicates en_US.po engine
count=$?
((error_count+=$count))
echo

###
phase
###

echo "All files valid."
