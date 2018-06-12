set( HTTP_PARSER_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}" )

set(
	HTTP_PARSER_SOURCES
	${HTTP_PARSER_SRC_DIR}/http_parser.c
)

add_library( http_parser OBJECT ${HTTP_PARSER_SOURCES} )

include_directories( SYSTEM "${CMAKE_CURRENT_LIST_DIR}" )

set_source_files_properties( ${HTTP_PARSER_SOURCES} PROPERTIES LANGUAGE "C" )

if( MSVC )
	set_source_files_properties( ${HTTP_PARSER_SOURCES} PROPERTIES COMPILE_FLAGS "/W0" )
endif()

if( CMAKE_COMPILER_IS_GNUCXX )
	set_source_files_properties( ${HTTP_PARSER_SOURCES} PROPERTIES COMPILE_FLAGS "-fstrict-aliasing" )
endif()