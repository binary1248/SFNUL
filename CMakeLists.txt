# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

cmake_minimum_required( VERSION 3.1 )

if( NOT CMAKE_BUILD_TYPE )
	set( CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)" FORCE )
endif()

project( SFNUL )

### USER INPUT ###

set( BUILD_SHARED_LIBS false CACHE BOOL "Build dynamic library." )
set( SFNUL_BUILD_EXAMPLES true CACHE BOOL "Build SFNUL examples." )
set( SFNUL_BUILD_SFML_EXAMPLES false CACHE BOOL "Build SFNUL examples that use SFML for visualisation." )
set( SFNUL_BUILD_DOC false CACHE BOOL "Generate the SFNUL API documentation." )
set( SFNUL_ENDIAN_SWAP false CACHE BOOL "Swap endianness of integral values. Enable if building for specific hardware." )
set( SFNUL_USE_STD_THREAD false CACHE BOOL "Use std::thread instead of platform-specific threading. (std::thread and std::recursive_mutex need to be fully supported)" )

if( MSVC )
	set( SFNUL_USE_STATIC_STD_LIBS false CACHE BOOL "True to link the runtime library statically, false to link them dynamically." )
endif()

# Find packages.
find_package( Threads QUIET )

if( NOT SFNUL_USE_STD_THREAD )
	if( CMAKE_USE_WIN32_THREADS_INIT )
		# We are using Win32 threads.
		message( STATUS "Using Win32 threads." )
	elseif( CMAKE_USE_PTHREADS_INIT )
		# We are using pthreads.
		message( STATUS "Using pthreads." )
	else()
		message( FATAL_ERROR "Not using std::thread and didn't find Win32 threads or pthreads. Other threading libraries are currently unsupported." )
	endif()
else()
	if( CMAKE_USE_WIN32_THREADS_INIT )
		# We are using Win32 std::thread implementation.
		message( STATUS "Using std::thread (Win32 implementation)." )
	elseif( CMAKE_USE_PTHREADS_INIT )
		# We are using pthreads std::thread implementation.
		message( STATUS "Using std::thread (pthreads implementation)." )
	else()
		message( STATUS "Using std::thread and wasn't able to detect underlying implementation. Errors may occur." )
	endif()

	add_definitions( -DSFNUL_USE_STD_THREAD )
endif()

# Make Windows happy.
if( WIN32 )
	set( BIN_EXTENSION ".exe" )
	set( SHARE_TARGET_DIR . )
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

set(
	HEADERS
	${INC_DIR}/SFNUL.hpp
	${INC_DIR}/SFNUL/Config.hpp
	${INC_DIR}/SFNUL/Concurrency.hpp
	${INC_DIR}/SFNUL/DataTypes.hpp
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
	${INC_DIR}/SFNUL/Utility.hpp
)

set(
	INTERNALHEADERS
	${SRC_DIR}/SFNUL/ConfigInternal.hpp
	${SRC_DIR}/SFNUL/IpAddressImpl.hpp
	${SRC_DIR}/SFNUL/MakeUnique.hpp
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
	${SRC_DIR}/SFNUL/Utility.cpp
)

set(
	ALL_FILES
	${HEADERS}
	${INTERNALHEADERS}
	${SOURCES}
)

file( READ "${INC_DIR}/SFNUL/Config.hpp" CONFIG_HPP )

string( REGEX MATCH ".*#define SFNUL_MAJOR_VERSION ([0-9]+).*#define SFNUL_MINOR_VERSION ([0-9]+).*" CONFIG_HPP "${CONFIG_HPP}" )
string( REGEX REPLACE ".*#define SFNUL_MAJOR_VERSION ([0-9]+).*" "\\1" SFNUL_MAJOR_VERSION "${CONFIG_HPP}" )
string( REGEX REPLACE ".*#define SFNUL_MINOR_VERSION ([0-9]+).*" "\\1" SFNUL_MINOR_VERSION "${CONFIG_HPP}" )

include_directories( ${INC_DIR} )
include_directories( ${SRC_DIR} )
include_directories( SYSTEM "${PROJECT_SOURCE_DIR}/extlibs/asio/asio/include" )
include_directories( SYSTEM "${PROJECT_SOURCE_DIR}/extlibs/botan/include" )
include_directories( SYSTEM "${PROJECT_SOURCE_DIR}/extlibs/http-parser" )

set_source_files_properties( ${SOURCES} PROPERTIES LANGUAGE "CXX" )

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if( MSVC )
	foreach( flag CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO )
		if( NOT ${flag} MATCHES "/MP" )
			set( ${flag} "${${flag}} /MP" )
		endif()
	endforeach()

	foreach( flag CMAKE_EXE_LINKER_FLAGS CMAKE_STATIC_LINKER_FLAGS CMAKE_SHARED_LINKER_FLAGS CMAKE_MODULE_LINKER_FLAGS )
		if( NOT ${flag} MATCHES "/ignore:4221" )
			set( ${flag} "${${flag}} /ignore:4221" )
		endif()
	endforeach()
endif()

# Static runtime linkage for our favorite compiler.
if( MSVC AND SFNUL_USE_STATIC_STD_LIBS )
	foreach( flag CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO )
		if( ${flag} MATCHES "/MD" )
			string( REGEX REPLACE "/MD" "/MT" ${flag} "${${flag}}" )
		endif()
	endforeach()
endif()

# Set reentrant for Botan
add_definitions( -D_REENTRANT )

add_subdirectory( "extlibs/botan" )
add_subdirectory( "extlibs/http-parser" )

# Add the library.
if( BUILD_SHARED_LIBS )
	add_library( sfnul SHARED $<TARGET_OBJECTS:botan> $<TARGET_OBJECTS:http_parser> ${ALL_FILES} )

	set_target_properties( sfnul PROPERTIES DEBUG_POSTFIX -d )

	if ( WIN32 AND CMAKE_COMPILER_IS_GNUCXX )
		set_target_properties( sfnul PROPERTIES PREFIX "" )
		set_target_properties( sfnul PROPERTIES IMPORT_SUFFIX ".a" )
	endif()
else()
	add_definitions( -DSFNUL_STATIC )

	add_library( sfnul $<TARGET_OBJECTS:botan> $<TARGET_OBJECTS:http_parser> ${ALL_FILES} )

	set_target_properties( sfnul PROPERTIES DEBUG_POSTFIX -s-d )
	set_target_properties( sfnul PROPERTIES RELEASE_POSTFIX -s )
	set_target_properties( sfnul PROPERTIES MINSIZEREL_POSTFIX -s )
endif()

# Set default compile flags for GCC
if( CMAKE_COMPILER_IS_GNUCXX )
	if( CMAKE_CXX_COMPILER MATCHES ".*clang[+][+]" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" )
		set_source_files_properties( ${SOURCES} PROPERTIES COMPILE_FLAGS "-stdlib=libc++ -Wall -Wextra -Wshadow -Wconversion -Wunreachable-code -Wredundant-decls -Wcast-align -Wfloat-equal -Wformat=2 -Wmissing-declarations -Woverlength-strings -pedantic" )
	else()
		set_source_files_properties( ${SOURCES} PROPERTIES COMPILE_FLAGS "-Wall -Wextra -Wshadow -Wconversion -Wunreachable-code -Wredundant-decls -Wcast-align -Wfloat-equal -Wformat=2 -Wmissing-declarations -Woverlength-strings -pedantic" )
	endif()
endif()

# Tell the compiler to export when necessary.
set_target_properties( sfnul PROPERTIES DEFINE_SYMBOL SFNUL_EXPORTS )

# Link to dependencies.
if( WIN32 )
	target_link_libraries( sfnul user32 ws2_32 mswsock )
elseif( APPLE )
	target_link_libraries( sfnul ${COREFOUNDATION_LIBRARY} )
endif()

if( CMAKE_THREAD_LIBS_INIT )
	target_link_libraries( sfnul ${CMAKE_THREAD_LIBS_INIT} )
endif()

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
	FILES cmake/FindSFNUL.cmake
	DESTINATION ./cmake
)

install(
	FILES README AUTHORS CHANGELOG LICENSE
	DESTINATION ${SHARE_TARGET_DIR}
)
