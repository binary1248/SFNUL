# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

cmake_minimum_required( VERSION 2.8 )

if( NOT CMAKE_BUILD_TYPE )
	set( CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)" FORCE )
endif()

project( SFNUL )

### USER INPUT ###

set( BUILD_SHARED_LIBS false CACHE BOOL "Build dynamic library." )
set( SFNUL_BUILD_EXAMPLES true CACHE BOOL "Build SFNUL examples." )
set( SFNUL_BUILD_SFML_EXAMPLES false CACHE BOOL "Build SFNUL examples that use SFML for visualisation." )
set( SFNUL_BUILD_DOC false CACHE BOOL "Generate the SFNUL API documentation." )

if( MSVC )
	set( SFNUL_USE_STATIC_STD_LIBS false CACHE BOOL "True to link the runtime library statically, false to link them dynamically." )
endif()

# Find packages.
find_package( Threads REQUIRED )

if( CMAKE_USE_WIN32_THREADS_INIT )
	# We are using Win32 threads.
	add_definitions( -DSFNUL_WIN32_THREADS )
	message( STATUS "Using Win32 threads." )
elseif( CMAKE_USE_PTHREADS_INIT )
	# We are using pthreads.
	add_definitions( -DSFNUL_PTHREADS )
	message( STATUS "Using pthreads." )
else()
	message( FATAL_ERROR "Didn't find Win32 threads or pthreads. Other threading libraries are currently unsupported." )
endif()

# Make Windows happy.
if( WIN32 )
	set( BIN_EXTENSION ".exe" )
	set( SHARE_TARGET_DIR . )
	add_definitions( -DWIN32 )
	add_definitions( -D_WIN32_WINNT=0x0501 )
else()
	set( SHARE_TARGET_DIR share/SFNUL )
endif()

# Make OS X happy.
if( APPLE )
	include_directories( /System/Library/Frameworks/CoreFoundation.framework/Headers )
	find_library( COREFOUNDATION_LIBRARY CoreFoundation )
	mark_as_advanced( COREFOUNDATION_LIBRARY )
endif()

set( LIBRARY_OUTPUT_PATH "${PROJECT_BINARY_DIR}/lib" )

set( SRC_DIR "${PROJECT_SOURCE_DIR}/src/" )
set( INC_DIR "${PROJECT_SOURCE_DIR}/include/" )
set( TROPICSSL_SRC_DIR "${PROJECT_SOURCE_DIR}/extlibs/tropicssl/library/" )
set( HTTP_PARSER_SRC_DIR "${PROJECT_SOURCE_DIR}/extlibs/http-parser/" )

set(
	HEADERS
	${INC_DIR}/SFNUL.hpp
	${INC_DIR}/SFNUL/Config.hpp
	${INC_DIR}/SFNUL/Concurrency.hpp
	${INC_DIR}/SFNUL/Endpoint.hpp
	${INC_DIR}/SFNUL/HTTP.hpp
	${INC_DIR}/SFNUL/HTTPClient.hpp
	${INC_DIR}/SFNUL/IpAddress.hpp
	${INC_DIR}/SFNUL/Link.hpp
	${INC_DIR}/SFNUL/Link.inl
	${INC_DIR}/SFNUL/Message.hpp
	${INC_DIR}/SFNUL/Message.inl
	${INC_DIR}/SFNUL/NetworkResource.hpp
	${INC_DIR}/SFNUL/ReliableTransport.hpp
	${INC_DIR}/SFNUL/Socket.hpp
	${INC_DIR}/SFNUL/SyncedObject.hpp
	${INC_DIR}/SFNUL/SyncedType.hpp
	${INC_DIR}/SFNUL/SyncedType.inl
	${INC_DIR}/SFNUL/Synchronizer.hpp
	${INC_DIR}/SFNUL/Synchronizer.inl
	${INC_DIR}/SFNUL/TcpListener.hpp
	${INC_DIR}/SFNUL/TcpListener.inl
	${INC_DIR}/SFNUL/TcpSocket.hpp
	${INC_DIR}/SFNUL/TlsConnection.hpp
	${INC_DIR}/SFNUL/TlsConnection.inl
	${INC_DIR}/SFNUL/Transport.hpp
	${INC_DIR}/SFNUL/UdpSocket.hpp
)

set(
	SOURCES
	${SRC_DIR}/SFNUL/Concurrency.cpp
	${SRC_DIR}/SFNUL/Endpoint.cpp
	${SRC_DIR}/SFNUL/HTTP.cpp
	${SRC_DIR}/SFNUL/HTTPClient.cpp
	${SRC_DIR}/SFNUL/IpAddress.cpp
	${SRC_DIR}/SFNUL/Link.cpp
	${SRC_DIR}/SFNUL/Message.cpp
	${SRC_DIR}/SFNUL/NetworkResource.cpp
	${SRC_DIR}/SFNUL/ReliableTransport.cpp
	${SRC_DIR}/SFNUL/Socket.cpp
	${SRC_DIR}/SFNUL/SyncedObject.cpp
	${SRC_DIR}/SFNUL/SyncedType.cpp
	${SRC_DIR}/SFNUL/Synchronizer.cpp
	${SRC_DIR}/SFNUL/TcpListener.cpp
	${SRC_DIR}/SFNUL/TcpSocket.cpp
	${SRC_DIR}/SFNUL/TlsConnection.cpp
	${SRC_DIR}/SFNUL/Transport.cpp
	${SRC_DIR}/SFNUL/UdpSocket.cpp
)

set(
	TROPICSSL_SOURCES
	${TROPICSSL_SRC_DIR}/aes.c
	${TROPICSSL_SRC_DIR}/arc4.c
	${TROPICSSL_SRC_DIR}/base64.c
	${TROPICSSL_SRC_DIR}/bignum.c
	${TROPICSSL_SRC_DIR}/camellia.c
	${TROPICSSL_SRC_DIR}/certs.c
	${TROPICSSL_SRC_DIR}/debug.c
	${TROPICSSL_SRC_DIR}/des.c
	${TROPICSSL_SRC_DIR}/dhm.c
	${TROPICSSL_SRC_DIR}/havege.c
	${TROPICSSL_SRC_DIR}/md2.c
	${TROPICSSL_SRC_DIR}/md4.c
	${TROPICSSL_SRC_DIR}/md5.c
	${TROPICSSL_SRC_DIR}/net.c
	${TROPICSSL_SRC_DIR}/padlock.c
	${TROPICSSL_SRC_DIR}/rsa.c
	${TROPICSSL_SRC_DIR}/sha1.c
	${TROPICSSL_SRC_DIR}/sha2.c
	${TROPICSSL_SRC_DIR}/sha4.c
	${TROPICSSL_SRC_DIR}/ssl_cli.c
	${TROPICSSL_SRC_DIR}/ssl_srv.c
	${TROPICSSL_SRC_DIR}/ssl_tls.c
	${TROPICSSL_SRC_DIR}/timing.c
	${TROPICSSL_SRC_DIR}/x509parse.c
	${TROPICSSL_SRC_DIR}/xtea.c
)

set(
	HTTP_PARSER_SOURCES
	${HTTP_PARSER_SRC_DIR}/http_parser.c
)

set(
	ALL_FILES
	${HTTP_PARSER_SOURCES}
	${TROPICSSL_SOURCES}
	${HEADERS}
	${SOURCES}
)

include_directories( ${INC_DIR} )
include_directories( SYSTEM "${PROJECT_SOURCE_DIR}/extlibs/asio/asio/include" )
include_directories( SYSTEM "${PROJECT_SOURCE_DIR}/extlibs/tropicssl/include" )
include_directories( SYSTEM "${PROJECT_SOURCE_DIR}/extlibs/http-parser" )

set_source_files_properties( ${SOURCES} PROPERTIES LANGUAGE "CXX" )
set_source_files_properties( ${TROPICSSL_SOURCES} PROPERTIES LANGUAGE "C" )
set_source_files_properties( ${HTTP_PARSER_SOURCES} PROPERTIES LANGUAGE "C" )

# Static runtime linkage for our favorite compiler.
if( MSVC AND SFNUL_USE_STATIC_STD_LIBS )
	foreach( flag CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO )
		if( ${flag} MATCHES "/MD" )
			string( REGEX REPLACE "/MD" "/MT" ${flag} "${${flag}}" )
		endif()
	endforeach()
endif()

# Add the library.
if( BUILD_SHARED_LIBS )
	add_library( sfnul SHARED ${ALL_FILES} )

	set_target_properties( sfnul PROPERTIES DEBUG_POSTFIX -d )

	if ( WIN32 AND CMAKE_COMPILER_IS_GNUCXX )
		set_target_properties( sfnul PROPERTIES PREFIX "" )
		set_target_properties( sfnul PROPERTIES IMPORT_SUFFIX ".a" )
	endif()
else()
	add_definitions( -DSFNUL_STATIC )

	add_library( sfnul ${ALL_FILES} )

	set_target_properties( sfnul PROPERTIES DEBUG_POSTFIX -s-d )
	set_target_properties( sfnul PROPERTIES RELEASE_POSTFIX -s )
	set_target_properties( sfnul PROPERTIES MINSIZEREL_POSTFIX -s )
endif()

# Set default compile flags for GCC
if( CMAKE_COMPILER_IS_GNUCXX )
	if( CMAKE_CXX_COMPILER MATCHES ".*clang[+][+]" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" )
		set_source_files_properties( ${SOURCES} PROPERTIES COMPILE_FLAGS "-stdlib=libc++ -std=c++11 -Wall -Wextra -Wshadow -Wconversion -Wunreachable-code -Wredundant-decls -pedantic" )
	else()
		set_source_files_properties( ${SOURCES} PROPERTIES COMPILE_FLAGS "-std=c++11 -Weffc++ -Wall -Wextra -Wshadow -Wconversion -Wunreachable-code -Wredundant-decls -pedantic" )
	endif()
	
	set_source_files_properties( ${TROPICSSL_SOURCES} PROPERTIES COMPILE_FLAGS "-D_FILE_OFFSET_BITS=64 -Wall -Wextra -Wformat=2 -fstrict-aliasing -Wstrict-aliasing -Wfloat-equal -Wundef -Wpointer-arith -Wbad-function-cast -Wcast-align -Wwrite-strings -Waddress -Waggregate-return -Wstrict-prototypes  -Wold-style-declaration -Wold-style-definition -Wmissing-parameter-type -Wmissing-field-initializers -Wmissing-noreturn -Wmissing-format-attribute -Wpacked -Wredundant-decls -Wnested-externs" )
	set_source_files_properties( ${HTTP_PARSER_SOURCES} PROPERTIES COMPILE_FLAGS "-Wall -Wextra -Wformat=2 -fstrict-aliasing -Wstrict-aliasing -Wfloat-equal -Wundef -Wpointer-arith -Wbad-function-cast -Wcast-align -Wwrite-strings -Waddress -Waggregate-return -Wstrict-prototypes  -Wold-style-declaration -Wold-style-definition -Wmissing-parameter-type -Wmissing-field-initializers -Wmissing-noreturn -Wmissing-format-attribute -Wpacked -Wredundant-decls -Wnested-externs" )
endif()

# Tell the compiler to export when necessary.
set_target_properties( sfnul PROPERTIES DEFINE_SYMBOL SFNUL_EXPORTS )

# Link to dependencies.
if( WIN32 )
	target_link_libraries( sfnul ws2_32 mswsock )
elseif( APPLE )
	target_link_libraries( sfnul ${COREFOUNDATION_LIBRARY} )
endif()

target_link_libraries( sfnul ${CMAKE_THREAD_LIBS_INIT} )

### EXAMPLES ###
add_subdirectory( "examples" )

### DOCUMENTATION ###

if( SFNUL_BUILD_DOC )
	add_subdirectory( "doc" )
endif()

### INSTALL TARGETS ###

install(
	TARGETS sfnul
	RUNTIME DESTINATION bin COMPONENT bin
	LIBRARY DESTINATION lib COMPONENT bin
	ARCHIVE DESTINATION lib COMPONENT dev
)

install(
	DIRECTORY include
	DESTINATION .
)

install(
	DIRECTORY extlibs/asio/asio/include
	DESTINATION .
	REGEX "^.*(\\.git|\\.gitignore|\\.am)$" EXCLUDE
)

install(
	DIRECTORY extlibs/tropicssl/include
	DESTINATION .
	REGEX "^.*(\\.git|\\.gitignore|\\.am)$" EXCLUDE
)

install(
	FILES extlibs/http-parser/http_parser.h
	DESTINATION ./include
)

install(
	FILES README AUTHORS CHANGELOG LICENSE
	DESTINATION ${SHARE_TARGET_DIR}
)
