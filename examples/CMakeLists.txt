# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

cmake_minimum_required( VERSION 2.8 )

function( build_example SAMPLE_NAME SOURCES )
	set_source_files_properties( ${SOURCES} PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -Wextra -Wshadow -Wconversion -Wunreachable-code -Wredundant-decls -Wcast-align -Wfloat-equal -Wformat=2 -Wmissing-declarations -Woverlength-strings -pedantic" )
	
	add_executable( ${SAMPLE_NAME} ${SOURCES} )
	target_link_libraries( ${SAMPLE_NAME} sfnul )
	
	if( WIN32 )
		target_link_libraries( ${SAMPLE_NAME} ws2_32 mswsock )
	endif()

	install(
		TARGETS ${SAMPLE_NAME}
		RUNTIME DESTINATION ${SHARE_TARGET_DIR}/examples COMPONENT examples
	)

	install(
		FILES ${SOURCES}
		DESTINATION ${SHARE_TARGET_DIR}/examples COMPONENT examples
	)
endfunction()

function( build_sfml_example SAMPLE_NAME SOURCES )
	set( SFML_STATIC_LIBRARIES false CACHE BOOL "Was SFML built as a static library?" )
	find_package( SFML 2 COMPONENTS graphics window system )
	
	# FindSFML was not run.
	if( NOT DEFINED FIND_SFML_LIB_SUFFIX )
		set( CMAKE_MODULE_PATH "" CACHE PATH "Directory containing FindSFML.cmake" )
		message( FATAL_ERROR "CMake couldn't find FindSFML.cmake. Did you remember to install SFML?\nOptionally, you can set the CMAKE_MODULE_PATH entry to the directory containing FindSFML.cmake.\nFindSFML.cmake will be searched for in there." )
	endif()

	# FindSFML couldn't find SFML.
	if( NOT SFML_FOUND )
		set( SFML_ROOT "" CACHE PATH "SFML root directory" )
		message( FATAL_ERROR "CMake couldn't find SFML. Set the SFML_ROOT entry to SFML's root directory (containing \"include\" and \"lib\" directories)." )
	endif()

	if( SFML_STATIC_LIBRARIES )
		# Even though FindSFML sets -DSFML_STATIC itself, better safe than sorry.
		add_definitions( -DSFML_STATIC )
	endif()
	
	include_directories( SYSTEM ${SFML_INCLUDE_DIR} )
	
	# Set default compile flags for GCC
	if( CMAKE_COMPILER_IS_GNUCXX )
		if( CMAKE_CXX_COMPILER MATCHES ".*clang[+][+]" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" )
			set_source_files_properties( ${SOURCES} PROPERTIES COMPILE_FLAGS "-stdlib=libc++ -std=c++11 -Wall -Wextra -Wshadow -Wconversion -Wunreachable-code -Wredundant-decls -Wcast-align -Wfloat-equal -Wformat=2 -Wmissing-declarations -Woverlength-strings -pedantic" )
		else()
			set_source_files_properties( ${SOURCES} PROPERTIES COMPILE_FLAGS "-std=c++11 -Weffc++ -Wall -Wextra -Wshadow -Wconversion -Wunreachable-code -Wredundant-decls -Wcast-align -Wfloat-equal -Wformat=2 -Wmissing-declarations -Woverlength-strings -pedantic" )
		endif()
	endif()
	
	add_executable( ${SAMPLE_NAME} ${SOURCES} )

	target_link_libraries( ${SAMPLE_NAME} sfnul ${SFML_LIBRARIES} ${OPENGL_glu_LIBRARY} ${OPENGL_gl_LIBRARY} )
	
	if( WIN32 )
		target_link_libraries( ${SAMPLE_NAME} ws2_32 mswsock )
	elseif( APPLE )
		target_link_libraries( ${SAMPLE_NAME} ${COREFOUNDATION_LIBRARY} )
	endif()

	install(
		TARGETS ${SAMPLE_NAME}
		RUNTIME DESTINATION ${SHARE_TARGET_DIR}/examples COMPONENT examples
	)

	install(
		FILES ${SOURCES}
		DESTINATION ${SHARE_TARGET_DIR}/examples COMPONENT examples
	)
endfunction()

if( SFNUL_BUILD_EXAMPLES )
	build_example( "HelloWorld" "HelloWorld.cpp" )
	build_example( "HTTPServer" "HTTPServer.cpp" )
	build_example( "HTTPQuery" "HTTPQuery.cpp" )
	build_example( "DNSQuery" "DNSQuery.cpp" )
	build_example( "EchoServer" "EchoServer.cpp" )
	build_example( "Loopback" "Loopback.cpp" )
	build_example( "TLSClient" "TLSClient.cpp" )
	build_example( "HTTPSServer" "HTTPSServer.cpp" )
	build_example( "Message" "Message.cpp" )
	build_example( "Link" "Link.cpp" )
	build_example( "Synchronization" "Synchronization.cpp" )
endif()

if( SFNUL_BUILD_SFML_EXAMPLES )
	build_sfml_example( "SynchronizationSFML" "SynchronizationSFML.cpp" )
endif()
