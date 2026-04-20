#!/bin/bash

cd "$(git rev-parse --show-toplevel)"

files=( libraries/Translation/*/en_US.po )
folders=( *src* )

wadstrings=
if [[ -n $DOOMWADDIR ]]
then
	pushd "${DOOMWADDIR}" >/dev/null
	md5sum ./*
	wadstrings="$( \
		strings -n 1 ./* \
			| grep -E '^[A-Z][a-zA-Z_0-9_]{0,35}$' \
			| sort -u \
	)"
	popd >/dev/null
fi

for file in "${files[@]}"
do
	[[ -f "${file}" ]] || continue
	echo "${file}"
	for token in $(grep msgid "${file}" | cut -d \" -f 2 | sort -u)
	do
		[[ -z "${token}" ]] && continue

		grep -qF "${token}" <<< "${wadstrings}" && continue

		found=0
		for folder in "${folders[@]}"
		do
			[[ -d "${folder}" ]] || continue
			grep -IrqF "${token}" "${folder}" || continue
			found=1
			break
		done
		[[ "${found}" == 0 ]] && printf "\t%s\n" "${token}"
	done
	echo
done
