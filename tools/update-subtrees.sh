#!/bin/bash

# usage:
# ./update-subtrees all
# ./update-subtrees component
# ./update-subtrees component commit

export GITROOT="$(git rev-parse --show-toplevel)"

if [[ "${1}" == "--allow-empty" ]]
then
	EMPTY_COMMIT_FLAG="--allow-empty"
	shift
fi
if [[ "${1}" == "add" || "${1}" == "pull" ]]
then
	operation="${1}"
	shift
else
	operation="pull"
fi

subtree_action() {
	name="${1}"; shift; dest="${1}"; shift; repo="${1}"; shift; ref="${1}"; shift

	if [[ "${1}" == "${name}" || "${1}" == "all" ]]
	then
		echo "fetching ${name}"
	else
		echo "${name} not specified, skipping" ; return
	fi
	shift

	[[ -n "${1}" ]] && ref="${1}"
	shift
	[[ -n "${1}" ]] && repo="${1}"
	shift

	# Exit script if there's any error
	set -e
	RANDOM_COMMIT_MESSAGE="${SRANDOM}"
	git -C "${GITROOT}" subtree "${operation}" --prefix="${dest}" "${repo}" "${ref}" --squash --message "${RANDOM_COMMIT_MESSAGE}"
	COMMIT_MESSAGE_TMP_FILE="$(mktemp)"
	trap 'set +e && rm "${COMMIT_MESSAGE_TMP_FILE}"' RETURN
	[[ "$(git log --format=%B -n 1)" != "${RANDOM_COMMIT_MESSAGE}" ]] && return
	if [[ "${operation}" == "add" ]]
	then
		printf "Add ${dest} from ${ref}\n\n" > "${COMMIT_MESSAGE_TMP_FILE}"
	else
		printf "Update ${dest} to ${ref}\n\n" > "${COMMIT_MESSAGE_TMP_FILE}"
	fi
	cat <(git log --format=%B -n 1 HEAD^2) >> "${COMMIT_MESSAGE_TMP_FILE}"
	git reset --soft HEAD^
	if [[ -z "$(git status --porcelain)" ]]
	then
		echo "Fetched '${name}' has different commit hash but no actual changes!"
		[[ -z "${EMPTY_COMMIT_FLAG}" ]] && return
	fi
	git commit ${EMPTY_COMMIT_FLAG} -F "${COMMIT_MESSAGE_TMP_FILE}"
}

subtree_action 'zwidget'     'libraries/ZWidget'     'https://github.com/UZDoom/ZWidget'     'legacy' "${@}"
subtree_action 'zmusic'      'libraries/ZMusic'      'https://github.com/UZDoom/ZMusic'      'trunk'  "${@}"
subtree_action 'translation' 'libraries/Translation' 'https://github.com/UZDoom/Translation' 'main'   "${@}"
subtree_action 'zvulkan'     'libraries/ZVulkan'     'https://github.com/UZDoom/ZVulkan'     'legacy' "${@}"
