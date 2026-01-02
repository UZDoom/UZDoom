cmake_minimum_required( VERSION 3.16 )
if (EMSCRIPTEN) #mock target
	file(WRITE zzipdir.c "")
	add_executable( zipdir zzipdir.c)
endif ()
if( NOT CMAKE_CROSSCOMPILING )
	include_directories( SYSTEM "${BZIP2_INCLUDE_DIR}" "${LZMA_INCLUDE_DIR}" )
	add_executable( zipdir
		zipdir.c )
	target_link_libraries( zipdir miniz ${BZIP2_LIBRARIES} lzma )
	set( CROSS_EXPORTS ${CROSS_EXPORTS} zipdir PARENT_SCOPE )
endif()
