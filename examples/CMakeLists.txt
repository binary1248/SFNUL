# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

cmake_minimum_required( VERSION 2.8 )

function( build_example SAMPLE_NAME SOURCES )
	# Set default compile flags for GCC
	if( CMAKE_COMPILER_IS_GNUCXX )
		if( CMAKE_CXX_COMPILER MATCHES ".*clang[+][+]" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" )
			set_source_files_properties( ${SOURCES} PROPERTIES COMPILE_FLAGS "-stdlib=libc++ -std=c++11 -Wall -Wshadow -Wconversion -Wunreachable-code -Wredundant-decls -Wcast-align -Wfloat-equal -Wformat=2 -Wmissing-declarations -Woverlength-strings -pedantic" )
		else()
			set_source_files_properties( ${SOURCES} PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -Wshadow -Wconversion -Wunreachable-code -Wredundant-decls -Wcast-align -Wfloat-equal -Wformat=2 -Wmissing-declarations -Woverlength-strings -pedantic" )
		endif()
	endif()
	
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
	include_directories( SYSTEM ${SFML_INCLUDE_DIR} )
	
	# Set default compile flags for GCC
	if( CMAKE_COMPILER_IS_GNUCXX )
		if( CMAKE_CXX_COMPILER MATCHES ".*clang[+][+]" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" )
			set_source_files_properties( ${SOURCES} PROPERTIES COMPILE_FLAGS "-stdlib=libc++ -std=c++11 -Wall -Wshadow -Wconversion -Wunreachable-code -Wredundant-decls -Wcast-align -Wfloat-equal -Wformat=2 -Wmissing-declarations -Woverlength-strings -pedantic" )
		else()
			set_source_files_properties( ${SOURCES} PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -Wshadow -Wconversion -Wunreachable-code -Wredundant-decls -Wcast-align -Wfloat-equal -Wformat=2 -Wmissing-declarations -Woverlength-strings -pedantic" )
		endif()
	endif()
	
	add_executable( ${SAMPLE_NAME} ${SOURCES} )

	target_link_libraries( ${SAMPLE_NAME} sfnul ${SFML_LIBRARIES} ${SFML_DEPENDENCIES} )
	
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
	# SFML_STATIC_LIBRARIES is used by FindSFML.cmake to select the correct library files to link to.
	set( SFML_STATIC_LIBRARIES false CACHE BOOL "Was SFML built as a static library?" )

	# A bit of non-trivial information.
	message( STATUS "If using SFML as a static library remember to set SFML_STATIC_LIBRARIES." )

	# Automatically grab SFML_ROOT from the environment if it exists, or force the user to specify it if it doesn't.
	set( SFML_ROOT "$ENV{SFML_ROOT}" CACHE PATH "SFML root directory." )

	if( "${SFML_ROOT}" STREQUAL "" )
		message( FATAL_ERROR "The SFML_ROOT environment variable was not found. Please set SFML_ROOT to the directory where SFML is located." )
	endif()

	# Add SFML's CMake Modules directory to module search path for finding FindSFML.cmake more easily.
	set( CMAKE_MODULE_PATH "${SFML_ROOT}/cmake/Modules/;${CMAKE_MODULE_PATH}" )
	
	find_package( SFML 2 REQUIRED COMPONENTS graphics window system )
	
	build_sfml_example( "SynchronizationSFML" "SynchronizationSFML.cpp" )
	build_sfml_example( "TestClient" "TestClient.cpp" )
	build_sfml_example( "TestServer" "TestServer.cpp" )
endif()
