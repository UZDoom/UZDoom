#!/usr/bin/cmake -P

# UpdatePlistVersion.cmake
#
# Runs as a POST_BUILD step to stamp the macOS bundle Info.plist
# with the version derived from git describe (or the GIT_DESCRIBE
# env var).  Follows the same pattern as UpdateRevision.cmake.

# Populate variable "Version" with X.Y.Z parsed from git describe.
# Falls back to "0.0.0" if anything goes wrong.
function(query_version_info)
	execute_process(
		COMMAND git rev-parse --is-inside-work-tree
		RESULT_VARIABLE is_git
		OUTPUT_QUIET
	)

	set(Version "0.0.0")

	if(DEFINED ENV{GIT_DESCRIBE})
		set(Tag "$ENV{GIT_DESCRIBE}")

		if(is_git EQUAL "0")
			message(STATUS "Version tag overridden by GIT_DESCRIBE env var")
		endif()
	elseif(is_git EQUAL "0")
		execute_process(
			COMMAND git describe --tags
			RESULT_VARIABLE Error
			OUTPUT_VARIABLE Tag
			ERROR_QUIET
			OUTPUT_STRIP_TRAILING_WHITESPACE
		)

		if(NOT "${Error}" STREQUAL "0")
			message(STATUS "No git tags found! Using fallback '${Version}'")
			set(Version "${Version}" PARENT_SCOPE)
			return()
		endif()
	else()
		message(STATUS "Not a git repo! Set version tag by setting GIT_DESCRIBE env var")
		set(Version "${Version}" PARENT_SCOPE)
		return()
	endif()

	string(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)" _match "${Tag}")
	if(NOT _match)
		message(STATUS "Cannot parse X.Y.Z from '${Tag}'! Using fallback '${Version}'")
	else()
		set(Version "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3}")
	endif()

	set(Version "${Version}" PARENT_SCOPE)
endfunction()

function(main)
	if(NOT DEFINED PLIST_FILE)
		message(FATAL_ERROR "PLIST_FILE not set. Pass -DPLIST_FILE=<path>")
	endif()

	query_version_info()
	message(STATUS "Plist bundle version: ${Version}")

	execute_process(
		COMMAND /usr/libexec/PlistBuddy
			-c "Set :CFBundleShortVersionString ${Version}"
			-c "Set :CFBundleVersion ${Version}"
			"${PLIST_FILE}"
		RESULT_VARIABLE _pb_result
	)
	if(NOT _pb_result EQUAL 0)
		message(FATAL_ERROR "PlistBuddy failed (${_pb_result})")
	endif()
endfunction()

main()
