#!/usr/bin/cmake -P

# UpdateRevision.cmake
#
# Public domain. This program uses git commands command to get
# various bits of repository status for a particular directory
# and writes it into a header file so that it can be used for a
# project's versioning.

# Boilerplate to return a variable from a function.
macro(ret_var VAR)
	set(${VAR} "${${VAR}}" PARENT_SCOPE)
endmacro()

# Populate variables "Hash", "Tag", and "Timestamp" with relevant information
# from source repository.  If anything goes wrong return something in "Error."
function(query_repo_info)
	# are we git?
	execute_process(
		COMMAND git rev-parse --is-inside-work-tree
		RESULT_VARIABLE is_git
		OUTPUT_QUIET
	)

	set(Description "unknown")
	set(Tag "unknown")
	set(Distance 0)
	set(Hash "0000000")
	string(TIMESTAMP Timestamp "%Y-%m-%d %H:%M:%S %z")

	if(DEFINED ENV{GIT_DESCRIBE})
		# from env
		set(Description "$ENV{GIT_DESCRIBE}")

		if (is_git EQUAL "0")
			message(STATUS "Version tag overridden by GIT_DESCRIBE env var")
		endif()

		# Extract hash from "...-gabcdef"
		string(REGEX MATCH "-g([0-9a-fA-F]+)" match_result "${Description}")
		if(match_result)
			set(Hash "${CMAKE_MATCH_1}")
		endif()
	elseif(is_git EQUAL "0")
		# from git
		execute_process(
			COMMAND git describe --tags --abbrev=0 --exclude x-* --always
			RESULT_VARIABLE Error
			OUTPUT_VARIABLE Temp
			ERROR_QUIET
			OUTPUT_STRIP_TRAILING_WHITESPACE
		)

		if(NOT "${Error}" STREQUAL "0")
			message(STATUS "No git tags found! Using fallback '${Description}'")
		else()
			set(Description "${Temp}")
			execute_process(
				COMMAND git rev-list "${Description}..HEAD" --count
				RESULT_VARIABLE Error
				OUTPUT_VARIABLE Temp
				ERROR_QUIET
				OUTPUT_STRIP_TRAILING_WHITESPACE
			)
			if("${Error}" STREQUAL "0")
				set(Distance "${Temp}")

				execute_process(
					COMMAND git status --porcelain
					OUTPUT_VARIABLE Temp
					ERROR_QUIET
					OUTPUT_STRIP_TRAILING_WHITESPACE
				)

				if(NOT "${Temp}" STREQUAL "")
					message(STATUS "Dirty build:\n${Temp}")
					math(EXPR Distance "(${Distance} * -1) - 1")
				endif()
			endif()
		endif()

		execute_process(
			COMMAND git describe --tags --abbrev=0 --exclude x-flathub-beta --always
			RESULT_VARIABLE Error
			OUTPUT_VARIABLE Temp
			ERROR_QUIET
			OUTPUT_STRIP_TRAILING_WHITESPACE
		)

		if(NOT "${Error}" STREQUAL "0")
			message(STATUS "No git tags found! Using fallback '${Description}'")
			set(Tag "${Description}")
		else()
			set(Tag "${Temp}")
		endif()

		execute_process(
			COMMAND git log -1 "--format=%aI;%H"
			RESULT_VARIABLE Error
			OUTPUT_VARIABLE Temp
			ERROR_QUIET
			OUTPUT_STRIP_TRAILING_WHITESPACE
		)

		if(NOT "${Error}" STREQUAL "0")
			message(STATUS "No git commits found! Using fallback '${Hash}'")
		else()
			string(REPLACE ";" ";" CommitInfo "${Temp}")
			list(GET CommitInfo 0 Timestamp)
			list(GET CommitInfo 1 Hash)
		endif()

		if (NOT ${Distance} STREQUAL "0")
			if ("${Distance}" LESS 0)
				math(EXPR Temp "(${Distance} + 1) * -1")
				set(Description "${Description}.${Temp}")
			else()
				set(Description "${Description}.${Distance}")
			endif()
			string(SUBSTRING ${Hash} 0 7 Temp)
			set(Description "${Description}+${Temp}")
			if ("${Distance}" LESS 0)
				set(Description "${Description}-m")
			endif()
		endif()

	else()
		message(STATUS "Not a git repo! Set version tag by setting GIT_DESCRIBE env var")
	endif()

	ret_var(Description)
	ret_var(Tag)
	ret_var(Distance)
	ret_var(Hash)
	ret_var(Timestamp)
endfunction()

function(main)
	if(NOT CMAKE_ARGC EQUAL 4) # cmake -P UpdateRevision.cmake <OutputFile>
		message("Usage: ${CMAKE_ARGV2} <path to gitinfo.h>")
		return()
	endif()
	set(OutputFile "${CMAKE_ARGV3}")

	get_filename_component(ScriptDir "${CMAKE_SCRIPT_MODE_FILE}" DIRECTORY)
	get_filename_component(ProjectDir "${CMAKE_SOURCE_DIR}/.." ABSOLUTE)

	query_repo_info()
	if(NOT Hash)
		message(FATAL_ERROR "Failed to get commit info: ${Error}")
	endif()

	configure_file("${ScriptDir}/gitinfo.h.in" "${OutputFile}" @ONLY)

	file(RELATIVE_PATH RelativeFile "${ProjectDir}" "${OutputFile}")
	message(STATUS "Revision ${RelativeFile}: ${Description}")
	message(STATUS "Revision ${RelativeFile}: ${Tag} | ${Distance} | ${Hash} | ${Timestamp}")
endfunction()

main()
