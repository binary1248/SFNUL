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
set( BOTAN_SRC_DIR "${PROJECT_SOURCE_DIR}/extlibs/botan/src/" )
set( HTTP_PARSER_SRC_DIR "${PROJECT_SOURCE_DIR}/extlibs/http-parser/" )

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
	BOTAN_SOURCES
	${BOTAN_SRC_DIR}/lib/algo_base/scan_name.cpp
	${BOTAN_SRC_DIR}/lib/algo_base/symkey.cpp
	${BOTAN_SRC_DIR}/lib/algo_factory/algo_factory.cpp
	${BOTAN_SRC_DIR}/lib/algo_factory/prov_weight.cpp
	${BOTAN_SRC_DIR}/lib/asn1/alg_id.cpp
	${BOTAN_SRC_DIR}/lib/asn1/asn1_alt_name.cpp
	${BOTAN_SRC_DIR}/lib/asn1/asn1_attribute.cpp
	${BOTAN_SRC_DIR}/lib/asn1/asn1_obj.cpp
	${BOTAN_SRC_DIR}/lib/asn1/asn1_oid.cpp
	${BOTAN_SRC_DIR}/lib/asn1/asn1_str.cpp
	${BOTAN_SRC_DIR}/lib/asn1/asn1_time.cpp
	${BOTAN_SRC_DIR}/lib/asn1/ber_dec.cpp
	${BOTAN_SRC_DIR}/lib/asn1/der_enc.cpp
	${BOTAN_SRC_DIR}/lib/asn1/oid_lookup/default.cpp
	${BOTAN_SRC_DIR}/lib/asn1/oid_lookup/oids.cpp
	${BOTAN_SRC_DIR}/lib/asn1/x509_dn.cpp
	${BOTAN_SRC_DIR}/lib/block/aes/aes.cpp
	${BOTAN_SRC_DIR}/lib/cert/x509/certstor.cpp
	${BOTAN_SRC_DIR}/lib/cert/x509/crl_ent.cpp
	${BOTAN_SRC_DIR}/lib/cert/x509/key_constraint.cpp
	${BOTAN_SRC_DIR}/lib/cert/x509/ocsp.cpp
	${BOTAN_SRC_DIR}/lib/cert/x509/ocsp_types.cpp
	${BOTAN_SRC_DIR}/lib/cert/x509/pkcs10.cpp
	${BOTAN_SRC_DIR}/lib/cert/x509/x509_ca.cpp
	${BOTAN_SRC_DIR}/lib/cert/x509/x509_crl.cpp
	${BOTAN_SRC_DIR}/lib/cert/x509/x509_ext.cpp
	${BOTAN_SRC_DIR}/lib/cert/x509/x509_obj.cpp
	${BOTAN_SRC_DIR}/lib/cert/x509/x509cert.cpp
	${BOTAN_SRC_DIR}/lib/cert/x509/x509opt.cpp
	${BOTAN_SRC_DIR}/lib/cert/x509/x509path.cpp
	${BOTAN_SRC_DIR}/lib/cert/x509/x509self.cpp
	${BOTAN_SRC_DIR}/lib/codec/base64/base64.cpp
	${BOTAN_SRC_DIR}/lib/codec/hex/hex.cpp
	${BOTAN_SRC_DIR}/lib/codec/pem/pem.cpp
	${BOTAN_SRC_DIR}/lib/constructs/cryptobox_psk/cryptobox_psk.cpp
	${BOTAN_SRC_DIR}/lib/constructs/srp6/srp6.cpp
	${BOTAN_SRC_DIR}/lib/constructs/srp6/srp6_files.cpp
	${BOTAN_SRC_DIR}/lib/credentials/credentials_manager.cpp
	${BOTAN_SRC_DIR}/lib/engine/core_engine/core_modes.cpp
	${BOTAN_SRC_DIR}/lib/engine/core_engine/def_pk_ops.cpp
	${BOTAN_SRC_DIR}/lib/engine/core_engine/def_powm.cpp
	${BOTAN_SRC_DIR}/lib/engine/core_engine/lookup_block.cpp
	${BOTAN_SRC_DIR}/lib/engine/core_engine/lookup_hash.cpp
	${BOTAN_SRC_DIR}/lib/engine/core_engine/lookup_mac.cpp
	${BOTAN_SRC_DIR}/lib/engine/core_engine/lookup_pbkdf.cpp
	${BOTAN_SRC_DIR}/lib/engine/core_engine/lookup_stream.cpp
	${BOTAN_SRC_DIR}/lib/engine/engine.cpp
	${BOTAN_SRC_DIR}/lib/filters/algo_filt.cpp
	${BOTAN_SRC_DIR}/lib/filters/basefilt.cpp
	${BOTAN_SRC_DIR}/lib/filters/buf_filt.cpp
	${BOTAN_SRC_DIR}/lib/filters/codec_filt/b64_filt.cpp
	${BOTAN_SRC_DIR}/lib/filters/codec_filt/hex_filt.cpp
	${BOTAN_SRC_DIR}/lib/filters/data_snk.cpp
	${BOTAN_SRC_DIR}/lib/filters/data_src.cpp
	${BOTAN_SRC_DIR}/lib/filters/filter.cpp
	${BOTAN_SRC_DIR}/lib/filters/out_buf.cpp
	${BOTAN_SRC_DIR}/lib/filters/pipe.cpp
	${BOTAN_SRC_DIR}/lib/filters/pipe_io.cpp
	${BOTAN_SRC_DIR}/lib/filters/pipe_rw.cpp
	${BOTAN_SRC_DIR}/lib/filters/secqueue.cpp
	${BOTAN_SRC_DIR}/lib/filters/threaded_fork.cpp
	${BOTAN_SRC_DIR}/lib/filters/transform_filter.cpp
	${BOTAN_SRC_DIR}/lib/hash/md5/md5.cpp
	${BOTAN_SRC_DIR}/lib/hash/mdx_hash/mdx_hash.cpp
	${BOTAN_SRC_DIR}/lib/hash/par_hash/par_hash.cpp
	${BOTAN_SRC_DIR}/lib/hash/sha1/sha160.cpp
	${BOTAN_SRC_DIR}/lib/hash/sha2_32/sha2_32.cpp
	${BOTAN_SRC_DIR}/lib/hash/sha2_64/sha2_64.cpp
	${BOTAN_SRC_DIR}/lib/kdf/kdf.cpp
	${BOTAN_SRC_DIR}/lib/kdf/kdf2/kdf2.cpp
	${BOTAN_SRC_DIR}/lib/kdf/prf_ssl3/prf_ssl3.cpp
	${BOTAN_SRC_DIR}/lib/kdf/prf_tls/prf_tls.cpp
	${BOTAN_SRC_DIR}/lib/libstate/entropy_srcs.cpp
	${BOTAN_SRC_DIR}/lib/libstate/global_state.cpp
	${BOTAN_SRC_DIR}/lib/libstate/init.cpp
	${BOTAN_SRC_DIR}/lib/libstate/libstate.cpp
	${BOTAN_SRC_DIR}/lib/libstate/lookup.cpp
	${BOTAN_SRC_DIR}/lib/mac/hmac/hmac.cpp
	${BOTAN_SRC_DIR}/lib/mac/mac.cpp
	${BOTAN_SRC_DIR}/lib/mac/ssl3mac/ssl3_mac.cpp
	${BOTAN_SRC_DIR}/lib/math/bigint/big_code.cpp
	${BOTAN_SRC_DIR}/lib/math/bigint/big_io.cpp
	${BOTAN_SRC_DIR}/lib/math/bigint/big_ops2.cpp
	${BOTAN_SRC_DIR}/lib/math/bigint/big_ops3.cpp
	${BOTAN_SRC_DIR}/lib/math/bigint/big_rand.cpp
	${BOTAN_SRC_DIR}/lib/math/bigint/bigint.cpp
	${BOTAN_SRC_DIR}/lib/math/bigint/divide.cpp
	${BOTAN_SRC_DIR}/lib/math/ec_gfp/point_gfp.cpp
	${BOTAN_SRC_DIR}/lib/math/mp/mp_asm.cpp
	${BOTAN_SRC_DIR}/lib/math/mp/mp_comba.cpp
	${BOTAN_SRC_DIR}/lib/math/mp/mp_karat.cpp
	${BOTAN_SRC_DIR}/lib/math/mp/mp_misc.cpp
	${BOTAN_SRC_DIR}/lib/math/mp/mp_monty.cpp
	${BOTAN_SRC_DIR}/lib/math/mp/mp_mulop.cpp
	${BOTAN_SRC_DIR}/lib/math/mp/mp_shift.cpp
	${BOTAN_SRC_DIR}/lib/math/numbertheory/dsa_gen.cpp
	${BOTAN_SRC_DIR}/lib/math/numbertheory/jacobi.cpp
	${BOTAN_SRC_DIR}/lib/math/numbertheory/make_prm.cpp
	${BOTAN_SRC_DIR}/lib/math/numbertheory/mp_numth.cpp
	${BOTAN_SRC_DIR}/lib/math/numbertheory/numthry.cpp
	${BOTAN_SRC_DIR}/lib/math/numbertheory/pow_mod.cpp
	${BOTAN_SRC_DIR}/lib/math/numbertheory/powm_fw.cpp
	${BOTAN_SRC_DIR}/lib/math/numbertheory/powm_mnt.cpp
	${BOTAN_SRC_DIR}/lib/math/numbertheory/primes.cpp
	${BOTAN_SRC_DIR}/lib/math/numbertheory/reducer.cpp
	${BOTAN_SRC_DIR}/lib/math/numbertheory/ressol.cpp
	${BOTAN_SRC_DIR}/lib/modes/aead/aead.cpp
	${BOTAN_SRC_DIR}/lib/modes/aead/gcm/gcm.cpp
	${BOTAN_SRC_DIR}/lib/modes/cbc/cbc.cpp
	${BOTAN_SRC_DIR}/lib/modes/mode_pad/mode_pad.cpp
	${BOTAN_SRC_DIR}/lib/pbe/get_pbe.cpp
	${BOTAN_SRC_DIR}/lib/pbkdf/pbkdf.cpp
	${BOTAN_SRC_DIR}/lib/pk_pad/eme.cpp
	${BOTAN_SRC_DIR}/lib/pk_pad/eme_pkcs1/eme_pkcs.cpp
	${BOTAN_SRC_DIR}/lib/pk_pad/emsa_pkcs1/emsa_pkcs1.cpp
	${BOTAN_SRC_DIR}/lib/pk_pad/get_pk_pad.cpp
	${BOTAN_SRC_DIR}/lib/pk_pad/hash_id/hash_id.cpp
	${BOTAN_SRC_DIR}/lib/pubkey/blinding.cpp
	${BOTAN_SRC_DIR}/lib/pubkey/dh/dh.cpp
	${BOTAN_SRC_DIR}/lib/pubkey/dl_algo/dl_algo.cpp
	${BOTAN_SRC_DIR}/lib/pubkey/dl_group/dl_group.cpp
	${BOTAN_SRC_DIR}/lib/pubkey/dl_group/named.cpp
	${BOTAN_SRC_DIR}/lib/pubkey/ec_group/ec_group.cpp
	${BOTAN_SRC_DIR}/lib/pubkey/ec_group/named.cpp
	${BOTAN_SRC_DIR}/lib/pubkey/ecc_key/ecc_key.cpp
	${BOTAN_SRC_DIR}/lib/pubkey/ecdh/ecdh.cpp
	${BOTAN_SRC_DIR}/lib/pubkey/ecdsa/ecdsa.cpp
	${BOTAN_SRC_DIR}/lib/pubkey/if_algo/if_algo.cpp
	${BOTAN_SRC_DIR}/lib/pubkey/keypair/keypair.cpp
	${BOTAN_SRC_DIR}/lib/pubkey/pk_algs.cpp
	${BOTAN_SRC_DIR}/lib/pubkey/pk_keys.cpp
	${BOTAN_SRC_DIR}/lib/pubkey/pkcs8.cpp
	${BOTAN_SRC_DIR}/lib/pubkey/pubkey.cpp
	${BOTAN_SRC_DIR}/lib/pubkey/rsa/rsa.cpp
	${BOTAN_SRC_DIR}/lib/pubkey/workfactor.cpp
	${BOTAN_SRC_DIR}/lib/pubkey/x509_key.cpp
	${BOTAN_SRC_DIR}/lib/rng/hmac_rng/hmac_rng.cpp
	${BOTAN_SRC_DIR}/lib/rng/rng.cpp
	${BOTAN_SRC_DIR}/lib/stream/ctr/ctr.cpp
	${BOTAN_SRC_DIR}/lib/stream/stream_cipher.cpp
	${BOTAN_SRC_DIR}/lib/tls/msg_cert_req.cpp
	${BOTAN_SRC_DIR}/lib/tls/msg_cert_verify.cpp
	${BOTAN_SRC_DIR}/lib/tls/msg_certificate.cpp
	${BOTAN_SRC_DIR}/lib/tls/msg_client_hello.cpp
	${BOTAN_SRC_DIR}/lib/tls/msg_client_kex.cpp
	${BOTAN_SRC_DIR}/lib/tls/msg_finished.cpp
	${BOTAN_SRC_DIR}/lib/tls/msg_hello_verify.cpp
	${BOTAN_SRC_DIR}/lib/tls/msg_next_protocol.cpp
	${BOTAN_SRC_DIR}/lib/tls/msg_server_hello.cpp
	${BOTAN_SRC_DIR}/lib/tls/msg_server_kex.cpp
	${BOTAN_SRC_DIR}/lib/tls/msg_session_ticket.cpp
	${BOTAN_SRC_DIR}/lib/tls/tls_alert.cpp
	${BOTAN_SRC_DIR}/lib/tls/tls_blocking.cpp
	${BOTAN_SRC_DIR}/lib/tls/tls_channel.cpp
	${BOTAN_SRC_DIR}/lib/tls/tls_ciphersuite.cpp
	${BOTAN_SRC_DIR}/lib/tls/tls_client.cpp
	${BOTAN_SRC_DIR}/lib/tls/tls_extensions.cpp
	${BOTAN_SRC_DIR}/lib/tls/tls_handshake_hash.cpp
	${BOTAN_SRC_DIR}/lib/tls/tls_handshake_io.cpp
	${BOTAN_SRC_DIR}/lib/tls/tls_handshake_state.cpp
	${BOTAN_SRC_DIR}/lib/tls/tls_heartbeats.cpp
	${BOTAN_SRC_DIR}/lib/tls/tls_policy.cpp
	${BOTAN_SRC_DIR}/lib/tls/tls_record.cpp
	${BOTAN_SRC_DIR}/lib/tls/tls_server.cpp
	${BOTAN_SRC_DIR}/lib/tls/tls_session.cpp
	${BOTAN_SRC_DIR}/lib/tls/tls_session_key.cpp
	${BOTAN_SRC_DIR}/lib/tls/tls_session_manager_memory.cpp
	${BOTAN_SRC_DIR}/lib/tls/tls_suite_info.cpp
	${BOTAN_SRC_DIR}/lib/tls/tls_version.cpp
	${BOTAN_SRC_DIR}/lib/utils/assert.cpp
	${BOTAN_SRC_DIR}/lib/utils/calendar.cpp
	${BOTAN_SRC_DIR}/lib/utils/charset.cpp
	${BOTAN_SRC_DIR}/lib/utils/cpuid.cpp
	${BOTAN_SRC_DIR}/lib/utils/datastor/datastor.cpp
	${BOTAN_SRC_DIR}/lib/utils/http_util/http_util.cpp
	${BOTAN_SRC_DIR}/lib/utils/parsing.cpp
	${BOTAN_SRC_DIR}/lib/utils/read_cfg.cpp
	${BOTAN_SRC_DIR}/lib/utils/semaphore.cpp
	${BOTAN_SRC_DIR}/lib/utils/version.cpp
	${BOTAN_SRC_DIR}/lib/utils/zero_mem.cpp
)

# Add Botan entropy sources
if( WIN32 )
	set(
		BOTAN_SOURCES
		${BOTAN_SOURCES}
		${BOTAN_SRC_DIR}/lib/entropy/win32_stats/es_win32.cpp
	)
	
	add_definitions( -DBOTAN_HAS_ENTROPY_SRC_WIN32=20131128 )
else()
	set(
		BOTAN_SOURCES
		${BOTAN_SOURCES}
		${BOTAN_SRC_DIR}/lib/entropy/unix_procs/unix_proc_sources.cpp
		${BOTAN_SRC_DIR}/lib/entropy/unix_procs/unix_procs.cpp
	)
	
	add_definitions( -DBOTAN_HAS_ENTROPY_SRC_UNIX_PROCESS_RUNNER=20131128 )
endif()

set(
	HTTP_PARSER_SOURCES
	${HTTP_PARSER_SRC_DIR}/http_parser.c
)

set(
	ALL_FILES
	${HTTP_PARSER_SOURCES}
	${BOTAN_SOURCES}
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
set_source_files_properties( ${BOTAN_SOURCES} PROPERTIES LANGUAGE "CXX" )
set_source_files_properties( ${HTTP_PARSER_SOURCES} PROPERTIES LANGUAGE "C" )

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
		set_source_files_properties( ${SOURCES} PROPERTIES COMPILE_FLAGS "-stdlib=libc++ -std=c++11 -Wall -Wextra -Wshadow -Wconversion -Wunreachable-code -Wredundant-decls -Wcast-align -Wfloat-equal -Wformat=2 -Wmissing-declarations -Woverlength-strings -pedantic" )
	else()
		set_source_files_properties( ${SOURCES} PROPERTIES COMPILE_FLAGS "-std=c++11 -Wall -Wextra -Wshadow -Wconversion -Wunreachable-code -Wredundant-decls -Wcast-align -Wfloat-equal -Wformat=2 -Wmissing-declarations -Woverlength-strings -pedantic" )
	endif()
	
	set_source_files_properties( ${BOTAN_SOURCES} PROPERTIES COMPILE_FLAGS "-std=c++11" )
	set_source_files_properties( ${HTTP_PARSER_SOURCES} PROPERTIES COMPILE_FLAGS "-fstrict-aliasing" )
endif()

# Tell the compiler to export when necessary.
set_target_properties( sfnul PROPERTIES DEFINE_SYMBOL SFNUL_EXPORTS )

# Link to dependencies.
if( WIN32 )
	target_link_libraries( sfnul ws2_32 mswsock )
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
