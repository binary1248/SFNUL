cmake_minimum_required( VERSION 2.8 )

if( NOT CMAKE_BUILD_TYPE )
	set( CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build (Debug or Release)" FORCE )
endif()

project( SFNUL )

### USER INPUT ###

set( BUILD_SHARED_LIBS false CACHE BOOL "Build dynamic library." )
set( SFNUL_BUILD_EXAMPLES true CACHE BOOL "Build SFNUL examples." )
set( SFNUL_BUILD_DOC false CACHE BOOL "Generate the SFNUL API documentation." )
set( SFML_STATIC_LIBRARIES false CACHE BOOL "Was SFML built as a static library?" )

if( SFML_STATIC_LIBRARIES AND BUILD_SHARED_LIBS )
	message( FATAL_ERROR "If you link to SFML statically you MUST link to SFNUL statically too." )
endif()

if( MSVC )
	set( SFNUL_USE_STATIC_STD_LIBS false CACHE BOOL "True to link the runtime library statically, false to link them dynamically." )
endif()

# Add SFML's CMake Modules directory from extlibs to module search path for
# finding FindSFML.cmake more easily.
set( CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${PROJECT_SOURCE_DIR}/extlibs/SFML/cmake/Modules/" )

# Find packages.
find_package( SFML 2 COMPONENTS network system )

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

set(
	SOURCES
	${INC_DIR}/SFNUL.hpp
	${INC_DIR}/SFNUL/Config.hpp
	${INC_DIR}/SFNUL/Endpoint.hpp
	${INC_DIR}/SFNUL/IpAddress.hpp
	${INC_DIR}/SFNUL/NetworkResource.hpp
	${INC_DIR}/SFNUL/NonCopyable.hpp
	${INC_DIR}/SFNUL/Socket.hpp
	${INC_DIR}/SFNUL/TcpListener.hpp
	${INC_DIR}/SFNUL/TcpSocket.hpp
	${INC_DIR}/SFNUL/UdpSocket.hpp
	${SRC_DIR}/SFNUL/Endpoint.cpp
	${SRC_DIR}/SFNUL/IpAddress.cpp
	${SRC_DIR}/SFNUL/NetworkResource.cpp
	${SRC_DIR}/SFNUL/Socket.cpp
	${SRC_DIR}/SFNUL/TcpListener.cpp
	${SRC_DIR}/SFNUL/TcpSocket.cpp
	${SRC_DIR}/SFNUL/UdpSocket.cpp
	
)

include_directories( ${INC_DIR} )
include_directories( SYSTEM ${SFML_INCLUDE_DIR} )
include_directories( SYSTEM "${PROJECT_SOURCE_DIR}/extlibs/asio/asio/include" )

set_source_files_properties( ${SOURCES} PROPERTIES LANGUAGE "CXX" )

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
	add_library( sfnul SHARED ${SOURCES} )

	set_target_properties( sfnul PROPERTIES DEBUG_POSTFIX -d )

	if ( WIN32 AND CMAKE_COMPILER_IS_GNUCXX )
		set_target_properties( sfnul PROPERTIES PREFIX "" )
		set_target_properties( sfnul PROPERTIES IMPORT_SUFFIX ".a" )
	endif()
else()
	add_definitions( -DSFNUL_STATIC )
	
	add_library( sfnul ${SOURCES} )

	set_target_properties( sfnul PROPERTIES DEBUG_POSTFIX -s-d )
	set_target_properties( sfnul PROPERTIES RELEASE_POSTFIX -s )
	set_target_properties( sfnul PROPERTIES MINSIZEREL_POSTFIX -s )
endif()

# Use asio in standalone header-only mode
add_definitions( -DASIO_STANDALONE )
add_definitions( -DASIO_HEADER_ONLY )

# Set default compile flags for GCC
if( CMAKE_COMPILER_IS_GNUCXX )
	add_definitions( -std=c++11 -Wall -Wextra -Weffc++ -Wshadow -Wconversion -Wunreachable-code -Wredundant-decls -pedantic )
endif()

# Tell the compiler to export when necessary.
set_target_properties( sfnul PROPERTIES DEFINE_SYMBOL SFNUL_EXPORTS )

# Link to dependencies.
if( WIN32 )
	target_link_libraries( sfnul ${SFML_NETWORK_LIBRARY} ${SFML_SYSTEM_LIBRARY} )
elseif( APPLE )
	target_link_libraries( sfnul ${COREFOUNDATION_LIBRARY} )
endif()

### EXAMPLES ###

if( SFNUL_BUILD_EXAMPLES )
	add_subdirectory( "examples" )
endif()

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
	REGEX "^.*(\\.gitignore|\\.am)$" EXCLUDE
)

install(
	FILES README AUTHORS CHANGELOG LICENSE COPYING COPYING.LESSER extlibs/asio/asio/LICENSE_1_0.txt
	DESTINATION ${SHARE_TARGET_DIR}
)
