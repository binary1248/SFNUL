# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# This script locates the SFNUL library.
#
# USAGE
#
# By default, the dynamic version of SFNUL will be found. To find the static
# one instead, you must set the SFNUL_STATIC_LIBRARIES variable to TRUE before
# calling find_package( SFNUL ). In that case SFNUL_STATIC will also be defined
# by this script. Example:
#
# set( SFNUL_STATIC_LIBRARIES TRUE )
# find_package( SFNUL )
#
# If SFNUL is not installed in a standard path, you can use the SFNULDIR or
# SFNUL_ROOT CMake (or environment) variables to tell CMake where to look for
# SFNUL.
#
#
# OUTPUT
#
# This script defines the following variables:
#   - SFNUL_LIBRARY_DEBUG:   the path to the debug library
#   - SFNUL_LIBRARY_RELEASE: the path to the release library
#   - SFNUL_LIBRARY:         the path to the library to link to
#   - SFNUL_FOUND:           true if the SFNUL library is found
#   - SFNUL_INCLUDE_DIR:     the path where SFNUL headers are located (the directory containing the SFNUL/Config.hpp file)
#
#
# EXAMPLE
#
# find_package( SFNUL REQUIRED )
# include_directories( ${SFNUL_INCLUDE_DIR} )
# add_executable( myapp ... )
# target_link_libraries( myapp ${SFNUL_LIBRARY} ... )

set( SFNUL_FOUND false )

if( SFNUL_STATIC_LIBRARIES )
	set( SFNUL_SUFFIX "-s" )
	add_definitions( -DSFNUL_STATIC )
else()
	set( SFNUL_SUFFIX "" )
endif()

find_path(
	SFNUL_INCLUDE_DIR
	SFNUL/Config.hpp
	PATH_SUFFIXES
		include
	PATHS
		/usr
		/usr/local
		${SFNULDIR}
		${SFNUL_ROOT}
		$ENV{SFNULDIR}
		$ENV{SFNUL_ROOT}
)

find_library(
	SFNUL_LIBRARY_RELEASE
	sfnul${SFNUL_SUFFIX}
	PATH_SUFFIXES
		lib
		lib64
	PATHS
		/usr
		/usr/local
		${SFNULDIR}
		${SFNUL_ROOT}
		$ENV{SFNULDIR}
		$ENV{SFNUL_ROOT}
)

find_library(
	SFNUL_LIBRARY_DEBUG
	sfnul${SFNUL_SUFFIX}-d
	PATH_SUFFIXES
		lib
		lib64
	PATHS
		/usr
		/usr/local
		${SFNULDIR}
		${SFNUL_ROOT}
		$ENV{SFNULDIR}
		$ENV{SFNUL_ROOT}
)

if( SFNUL_LIBRARY_RELEASE AND SFNUL_LIBRARY_DEBUG )
	set( SFNUL_LIBRARY debug ${SFNUL_LIBRARY_DEBUG} optimized ${SFNUL_LIBRARY_RELEASE} )
endif()

if( SFNUL_LIBRARY_RELEASE AND NOT SFNUL_LIBRARY_DEBUG )
	set( SFNUL_LIBRARY_DEBUG ${SFNUL_LIBRARY_RELEASE} )
	set( SFNUL_LIBRARY ${SFNUL_LIBRARY_RELEASE} )
endif()

if( SFNUL_LIBRARY_DEBUG AND NOT SFNUL_LIBRARY_RELEASE )
	set( SFNUL_LIBRARY_RELEASE ${SFNUL_LIBRARY_DEBUG} )
	set( SFNUL_LIBRARY ${SFNUL_LIBRARY_DEBUG} )
endif()

if( NOT SFNUL_INCLUDE_DIR OR NOT SFNUL_LIBRARY )
	message( FATAL_ERROR "SFNUL not found." )
else()
	set( SFNUL_FOUND true )
	message( "SFNUL found: ${SFNUL_INCLUDE_DIR}" )
endif()
