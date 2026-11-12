#!/usr/bin/cmake -P

# Stamp macOS bundle Info.plist with version from gitinfo.h.
# Called as POST_BUILD with -DGITINFO_H=... -DPLIST_FILE=...

if(NOT EXISTS "${GITINFO_H}")
	message(STATUS "'${GITINFO_H}' not found, skipping plist version stamp")
	return()
endif()

file(READ "${GITINFO_H}" _gitinfo)
string(REGEX MATCH "GIT_DESCRIPTION \"([^\"]*)\"" _match "${_gitinfo}")
set(_tag "${CMAKE_MATCH_1}")

string(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)" _match "${_tag}")
if(NOT _match)
	message(STATUS "Git tag '${_tag}' has no X.Y.Z version, skipping plist stamp")
	return()
endif()
set(Version "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3}")

message(STATUS "Plist bundle version: ${Version}")
execute_process(COMMAND /usr/libexec/PlistBuddy
	-c "Set :CFBundleShortVersionString ${Version}"
	-c "Set :CFBundleVersion ${Version}"
	"${PLIST_FILE}"
)
