add_pk3(uzdoom.pk3 ${CMAKE_CURRENT_SOURCE_DIR}/static)

generate_language_files(uzdoom_pk3 common static/language.0)
generate_language_files(uzdoom_pk3 zdoom_engine_strings static/language.csv)

# insert appstream data into uzdoom.pk3, for release notes
add_custom_target(
	copy_metadata
	COMMAND ${CMAKE_COMMAND} -E copy
		"${CMAKE_CURRENT_SOURCE_DIR}/../src/posix/freedesktop/org.zdoom.UZDoom.metainfo.xml"
		"${CMAKE_CURRENT_SOURCE_DIR}/static/meta.xml"
	COMMAND ${CMAKE_COMMAND} -E copy
		"${CMAKE_CURRENT_SOURCE_DIR}/../CONTRIBUTORS"
		"${CMAKE_CURRENT_SOURCE_DIR}/static/contributors.txt"
)
add_dependencies(uzdoom_pk3 copy_metadata)
add_custom_command(
	TARGET uzdoom_pk3
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E remove
		"${CMAKE_CURRENT_SOURCE_DIR}/static/meta.xml"
	COMMAND ${CMAKE_COMMAND} -E remove
		"${CMAKE_CURRENT_SOURCE_DIR}/static/contributors.txt"
)
