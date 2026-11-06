cmake_minimum_required( VERSION 3.16 )

if( NOT CMAKE_CROSSCOMPILING )
	include_directories( SYSTEM "${BZIP2_INCLUDE_DIR}" "${LZMA_INCLUDE_DIR}" )
	add_executable( zipdir
		zipdir.c )

	set( ZIPDIR_LIBS miniz ${BZIP2_LIBRARIES} lzma )
	if( TARGET musl-fts OR MUSL_FTS_FOUND )
		list( APPEND ZIPDIR_LIBS fts )
	endif()

	target_link_libraries( zipdir ${ZIPDIR_LIBS} )
	set( CROSS_EXPORTS ${CROSS_EXPORTS} zipdir PARENT_SCOPE )
endif()
