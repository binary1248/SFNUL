# This script locates the SFNUL library
# ------------------------------------
#
# Usage
# -----
#
# You can enforce a specific version, one of either MAJOR.MINOR or only MAJOR.
# If nothing is specified, the version won't be checked i.e. any version will
# be accepted. SFNUL does not consist of multiple components, so specifying
# COMPONENTS is not required.
#
# Example:
#   find_package( SFNUL )     // no specific version
#   find_package( SFNUL 0.2 ) // version 0.2 or greater
#
# By default, the dynamic version of SFNUL will be found. To find the static
# version instead, you must set the SFNUL_STATIC_LIBRARIES variable to TRUE
# before calling find_package( SFNUL ). In that case, SFNUL_STATIC will also be
# defined by this script.
#
# Example:
#   set( SFNUL_STATIC_LIBRARIES TRUE )
#   find_package( SFNUL )
#
# Since you have to link all of SFNUL's dependencies when you link SFNUL
# statically, the variable SFNUL_DEPENDENCIES is also defined. See below
# for a detailed description.
#
# On Mac OS X if SFNUL_STATIC_LIBRARIES is not set to TRUE then by default CMake
# will search for frameworks unless CMAKE_FIND_FRAMEWORK is set to "NEVER".
# Please refer to CMake documentation for more details.
# Moreover, keep in mind that SFNUL frameworks are only available as release
# libraries unlike dylibs which are available for both release and debug modes.
#
# If SFNUL is not installed in a standard path, you can use the SFNUL_ROOT
# CMake (or environment) variable to tell CMake where to look for SFNUL.
#
# Output
# ------
#
# This script defines the following variables:
#   - SFNUL_LIBRARY_DEBUG:   the path to the debug library
#   - SFNUL_LIBRARY_RELEASE: the path to the release library
#   - SFNUL_LIBRARY:         the path to the library to link to
#   - SFNUL_FOUND:           TRUE if the SFNUL library is found
#   - SFNUL_INCLUDE_DIR:     the path where SFNUL headers are located (the directory containing the SFNUL/Config.hpp file)
#   - SFNUL_DEPENDENCIES:    the list of libraries SFNUL depends on, in case of static linking
#
# Example (dynamic linking):
#   find_package( SFNUL REQUIRED )
#   include_directories( ${SFNUL_INCLUDE_DIR} )
#   add_executable( myapp ... )
#   target_link_libraries( myapp ${SFNUL_LIBRARY} ... )
#
# Example (static linking):
#   set( SFNUL_STATIC_LIBRARIES TRUE )
#   find_package( SFNUL REQUIRED )
#   include_directories( ${SFNUL_INCLUDE_DIR} )
#   add_executable( myapp ... )
#   target_link_libraries( myapp ${SFNUL_LIBRARY} ${SFNUL_DEPENDENCIES} ... )

if( SFNUL_STATIC_LIBRARIES )
	add_definitions( -DSFNUL_STATIC )
endif()

set(
	SFNUL_SEARCH_PATHS
	${SFNUL_ROOT}
	$ENV{SFNUL_ROOT}
	~/Library/Frameworks
	/Library/Frameworks
	/usr/local
	/usr
	/sw
	/opt/local
	/opt/csw
	/opt
)

find_path(
	SFNUL_INCLUDE_DIR
	SFNUL/Config.hpp
	PATH_SUFFIXES
		include
	PATHS
		${SFNUL_SEARCH_PATHS}
)

set( SFNUL_VERSION_OK true )
if( SFNUL_FIND_VERSION AND SFNUL_INCLUDE_DIR )
	file( READ "${SFNUL_INCLUDE_DIR}/SFNUL/Config.hpp" SFNUL_CONFIG_HPP )
	
	string( REGEX MATCH ".*#define SFNUL_MAJOR_VERSION ([0-9]+).*#define SFNUL_MINOR_VERSION ([0-9]+).*" SFNUL_CONFIG_HPP "${SFNUL_CONFIG_HPP}" )
	string( REGEX REPLACE ".*#define SFNUL_MAJOR_VERSION ([0-9]+).*" "\\1" SFNUL_VERSION_MAJOR "${SFNUL_CONFIG_HPP}" )
	string( REGEX REPLACE ".*#define SFNUL_MINOR_VERSION ([0-9]+).*" "\\1" SFNUL_VERSION_MINOR "${SFNUL_CONFIG_HPP}" )
	
	math( EXPR SFNUL_REQUESTED_VERSION "${SFNUL_FIND_VERSION_MAJOR} * 100 + ${SFNUL_FIND_VERSION_MINOR}" )

	if( SFNUL_VERSION_MAJOR OR SFNUL_VERSION_MINOR )
		math( EXPR SFNUL_VERSION "${SFNUL_VERSION_MAJOR} * 100 + ${SFNUL_VERSION_MINOR}" )

		if( SFNUL_VERSION LESS SFNUL_REQUESTED_VERSION )
			set( SFNUL_VERSION_OK false )
		endif()
	else()
		# SFNUL version is < 0.2
		if( SFNUL_REQUESTED_VERSION GREATER 2 )
			set( SFNUL_VERSION_OK false )
			set( SFNUL_VERSION_MAJOR 0 )
			set( SFNUL_VERSION_MINOR x )
		endif()
	endif()
endif()

find_library(
	SFNUL_LIBRARY_DYNAMIC_RELEASE
	NAMES
		SFNUL
	PATH_SUFFIXES
		lib
		lib64
	PATHS
		${SFNUL_SEARCH_PATHS}
)

find_library(
	SFNUL_LIBRARY_DYNAMIC_DEBUG
	NAMES
		SFNUL-d
	PATH_SUFFIXES
		lib
		lib64
	PATHS
		${SFNUL_SEARCH_PATHS}
)

find_library(
	SFNUL_LIBRARY_STATIC_RELEASE
	NAMES
		SFNUL-s
	PATH_SUFFIXES
		lib
		lib64
	PATHS
		${SFNUL_SEARCH_PATHS}
)

find_library(
	SFNUL_LIBRARY_STATIC_DEBUG
	NAMES
		SFNUL-s-d
	PATH_SUFFIXES
		lib
		lib64
	PATHS
		${SFNUL_SEARCH_PATHS}
)

if( SFNUL_STATIC_LIBRARIES )
	if( SFNUL_LIBRARY_STATIC_RELEASE )
		set( SFNUL_LIBRARY_RELEASE ${SFNUL_LIBRARY_STATIC_RELEASE} )
	endif()
	if( SFNUL_LIBRARY_STATIC_DEBUG )
		set( SFNUL_LIBRARY_DEBUG ${SFNUL_LIBRARY_STATIC_DEBUG} )
	endif()
else()
	if( SFNUL_LIBRARY_DYNAMIC_RELEASE )
		set( SFNUL_LIBRARY_RELEASE ${SFNUL_LIBRARY_DYNAMIC_RELEASE} )
	endif()
	if( SFNUL_LIBRARY_DYNAMIC_DEBUG )
		set( SFNUL_LIBRARY_DEBUG ${SFNUL_LIBRARY_DYNAMIC_DEBUG} )
	endif()
endif()

mark_as_advanced(
	SFNUL_LIBRARY_STATIC_RELEASE
	SFNUL_LIBRARY_STATIC_DEBUG
	SFNUL_LIBRARY_DYNAMIC_RELEASE
	SFNUL_LIBRARY_DYNAMIC_DEBUG
)

if( SFNUL_LIBRARY_RELEASE OR SFNUL_LIBRARY_DEBUG )
	if( SFNUL_LIBRARY_RELEASE AND SFNUL_LIBRARY_DEBUG )
		set( SFNUL_LIBRARY debug ${SFNUL_LIBRARY_DEBUG} optimized ${SFNUL_LIBRARY_RELEASE} )
	elseif( SFNUL_LIBRARY_DEBUG AND NOT SFNUL_LIBRARY_RELEASE )
		set( SFNUL_LIBRARY_RELEASE ${SFNUL_LIBRARY_DEBUG} )
		set( SFNUL_LIBRARY ${SFNUL_LIBRARY_DEBUG} )
	elseif( SFNUL_LIBRARY_RELEASE AND NOT SFNUL_LIBRARY_DEBUG )
		set( SFNUL_LIBRARY_DEBUG ${SFNUL_LIBRARY_RELEASE} )
		set( SFNUL_LIBRARY ${SFNUL_LIBRARY_RELEASE} )
	endif()

	set( SFNUL_FOUND true )
else()
	set( SFNUL_LIBRARY "" )
	set( SFNUL_FOUND false )
endif()

mark_as_advanced(
	SFNUL_LIBRARY
	SFNUL_LIBRARY_RELEASE
	SFNUL_LIBRARY_DEBUG
)

if( SFNUL_STATIC_LIBRARIES )
	set( SFNUL_DEPENDENCIES )
	set( SFNUL_MISSING_DEPENDENCIES )

	find_package( Threads QUIET )

	if( CMAKE_THREAD_LIBS_INIT )
		set( SFNUL_DEPENDENCIES ${SFNUL_DEPENDENCIES} ${CMAKE_THREAD_LIBS_INIT} )
	endif()

	if( ${CMAKE_SYSTEM_NAME} MATCHES "Windows" )
		set( SFNUL_DEPENDENCIES ${SFNUL_DEPENDENCIES} "ws2_32" )
	elseif( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
		find_library( COREFOUNDATION_LIBRARY CoreFoundation )
		
		if( ${COREFOUNDATION_LIBRARY} STREQUAL "COREFOUNDATION_LIBRARY-NOTFOUND" )
			unset( COREFOUNDATION_LIBRARY )
			set( SFNUL_MISSING_DEPENDENCIES "${SFNUL_MISSING_DEPENDENCIES} COREFOUNDATION_LIBRARY" )
		endif()
		
		mark_as_advanced( COREFOUNDATION_LIBRARY )
		
		set( SFNUL_DEPENDENCIES ${SFNUL_DEPENDENCIES} ${COREFOUNDATION_LIBRARY} )
	endif()
endif()

if( NOT SFNUL_INCLUDE_DIR OR NOT SFNUL_LIBRARY )
	set( SFNUL_FOUND false )
endif()

if( NOT SFNUL_FOUND )
	set( FIND_SFNUL_ERROR "SFNUL not found." )
elseif( NOT SFNUL_VERSION_OK )
	set( FIND_SFNUL_ERROR "SFNUL found but version too low, requested: ${SFNUL_FIND_VERSION}, found: ${SFNUL_VERSION_MAJOR}.${SFNUL_VERSION_MINOR}" )
	set( SFNUL_FOUND false )
elseif( SFNUL_STATIC_LIBRARIES AND SFNUL_MISSING_DEPENDENCIES )
	set( FIND_SFNUL_ERROR "SFNUL found but some of its dependencies are missing: ${SFNUL_MISSING_DEPENDENCIES}" )
	set( SFNUL_FOUND false )
endif()

if( NOT SFNUL_FOUND )
	if( SFNUL_FIND_REQUIRED )
		message( FATAL_ERROR ${FIND_SFNUL_ERROR} )
	elseif( NOT SFNUL_FIND_QUIETLY )
		message( "${FIND_SFNUL_ERROR}" )
	endif()
else()
	if( SFNUL_FIND_VERSION )
		message( STATUS "Found SFNUL version ${SFNUL_VERSION_MAJOR}.${SFNUL_VERSION_MINOR} in ${SFNUL_INCLUDE_DIR}" )
	else()
		message( STATUS "Found SFNUL in ${SFNUL_INCLUDE_DIR}" )
	endif()
endif()
