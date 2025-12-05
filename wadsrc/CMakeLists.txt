add_pk3(uzdoom.pk3 ${CMAKE_CURRENT_SOURCE_DIR}/static)

generate_language_files(uzdoom_pk3 static/language.0
	engine/common)
generate_language_files(uzdoom_pk3 static/language.csv
	engine/zdoom)

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
