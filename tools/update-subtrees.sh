#!/bin/bash

# usage:
# ./update-subtrees all
# ./update-subtrees component
# ./update-subtrees component commit

export GITROOT="$(git rev-parse --show-toplevel)"

pull() {
	name=${1}; shift; dest=${1}; shift; repo=${1}; shift; ref=${1}; shift

	if [[ "${1}" == "${name}" || "${1}" == "all" ]]
	then
		echo "fetching ${name}"
	else
		echo "${name} not specified, skipping" ; return
	fi
	shift

	[[ -z ${1} ]] || ref=${1}
	shift

	git -C $GITROOT subtree pull --prefix="${dest}" "${repo}" "${ref}" \
		--squash --message "Update ${dest} to ${ref}" || exit
}

pull 'zwidget' 'libraries/ZWidget' 'https://github.com/UZDoom/ZWidget' 'trunk' "${@}"
pull 'zmusic'  'libraries/ZMusic'  'https://github.com/UZDoom/ZMusic'  'trunk' "${@}"
