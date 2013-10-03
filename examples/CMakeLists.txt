# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

cmake_minimum_required( VERSION 2.8 )

function( build_example SAMPLE_NAME SOURCES )
	find_package( SFML 2 COMPONENTS graphics window network system )
	
	set_source_files_properties( ${SOURCES} PROPERTIES COMPILE_FLAGS "-std=c++11 -Weffc++" )
	
	add_executable( ${SAMPLE_NAME} ${SOURCES} )
	target_link_libraries( ${SAMPLE_NAME} sfnul ${SFML_GRAPHICS_LIBRARY} ${SFML_WINDOW_LIBRARY} ${SFML_NETWORK_LIBRARY} ${SFML_SYSTEM_LIBRARY} ${OPENGL_glu_LIBRARY} ${OPENGL_gl_LIBRARY} )
	
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

build_example( "HelloWorld" "HelloWorld.cpp" )
build_example( "HTTPServer" "HTTPServer.cpp" )
build_example( "HTTPQuery" "HTTPQuery.cpp" )
build_example( "DNSQuery" "DNSQuery.cpp" )
build_example( "EchoServer" "EchoServer.cpp" )
build_example( "Loopback" "Loopback.cpp" )
build_example( "TLSClient" "TLSClient.cpp" )
build_example( "HTTPSServer" "HTTPSServer.cpp" )
