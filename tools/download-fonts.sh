#!/bin/bash

# source:
# https://github.com/majodev/google-webfonts-helper?tab=readme-ov-file#json-api
#
# used for src/widgets/widgetresourcedata.cpp
#
# Basic usage:
# download-fonts.sh wad_root font_path font_name ...
#
# Example:
# tools/download-fonts.sh wadsrc/static ui/noto 'Noto Sans' 'Noto Sans Armenian' 'Noto Sans Georgian' 'Noto Sans JP' 'Noto Sans KR'

API='https://gwfh.mranftl.com/api/fonts/%s'
FALLBACK='https://fonts.google.com/specimen/%s'

EXT='ttf'
COMMON="download=zip&variants=regular&formats=${EXT}"

GITROOT="$(git rev-parse --show-toplevel)" || exit
OUTDIR=$(realpath -m "${GITROOT}/${1}") && shift || exit
WADDIR="${1}" && shift || exit
OUTDIR=$(realpath -m "${OUTDIR}/${WADDIR}") || exit
if ! [ -d "${OUTDIR}" ]
then
	printf 'Not a valid folder: "%s"\n' "${OUTDIR}" >&2
	exit 1
fi

slugs=()
requests=()

failed=0
index=0
for font in "${@}"
do
	slug="$(tr '[A-Z0-9 ]' [a-z0-9-] <<< "${font}")"

	if ! [[ "${slug}" =~ ^[a-z0-9-]+$ ]]
	then
		printf 'Unable to create slug for "%s"\n' "${font}" >&2
		((failed++))
		continue
	fi

	font="$(tr ' ' '+' <<< "${font}")"

	printf -v url "${API}" "${slug}"
	printf "Fetching %s\n" "$url"
	data=$(curl -q "${url}" | jq)

	if [ $? != 0 ]
	then
		printf -v url "${FALLBACK}" "${font}"
		printf "Unable to download data for '%s'. Try checking %s\n" "${slug}" "${url}" >&2
		((failed++))
		continue
	fi

	printf -v request "%s?%s" "${slug}" "${COMMON}"
	if subset=$(jq -r '.subsets | join(",")' <<< "${data}")
	then
		printf -v request "%s&subsets=%s" "${request}" "${subset}"
	fi

	slugs[$index]="${slug}"
	requests[$index]="${request}"

	((index++))
done

if [[ "${failed}" > 0 ]]
then
	printf "Encountered %s issues. Exiting\n" "${failed}" >&2
	exit 1
fi

tempdir=$(mktemp -d)
trap '{ rm -rf -- "${tempdir}"; }' EXIT

zipfile="${tempdir}/temp.zip"
for (( i=0; i < $index; i++ ))
do
	printf -v url "${API}" "${requests[$i]}"
	curl "${url}" -o "${zipfile}"
	unzip "${zipfile}" -d "${tempdir}"
	mv "${tempdir}"/*."${EXT}" "${OUTDIR}/${slugs[$i]}.${EXT}"
	rm -r "${tempdir}"/*
done

printf "\nDone! The following files were updated: "
files=()
for slug in "${slugs[@]}"
do
	printf '%s/%s.%s\n' "${WADDIR}" "${slug}" "${EXT}"
done | jq -R -s 'split("\n") | map(select(length > 0))'
