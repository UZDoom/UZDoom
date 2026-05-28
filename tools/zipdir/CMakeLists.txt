cmake_minimum_required( VERSION 3.16 )

if( NOT CMAKE_CROSSCOMPILING OR EMSCRIPTEN )
	include_directories( SYSTEM "${BZIP2_INCLUDE_DIR}" "${LZMA_INCLUDE_DIR}" )
	add_executable( zipdir
		zipdir.c )
	target_link_libraries( zipdir miniz ${BZIP2_LIBRARIES} lzma )
	if(EMSCRIPTEN)
		target_link_options(zipdir PRIVATE
				-sSTACK_SIZE=8MB
				-sALLOW_MEMORY_GROWTH
				-sEXIT_RUNTIME
				-sENVIRONMENT=node
				-sNODERAWFS
		)
	endif()
	set( CROSS_EXPORTS ${CROSS_EXPORTS} zipdir PARENT_SCOPE )
endif()
