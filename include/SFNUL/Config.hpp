/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#if defined( _WIN32 ) || defined( __WIN32__ )
	#define SFNUL_SYSTEM_WINDOWS
	#define WIN32_LEAN_AND_MEAN

	#ifndef NOMINMAX
		#define NOMINMAX
	#endif
#endif

#if defined( SFNUL_SYSTEM_WINDOWS ) && !defined( SFNUL_STATIC )
	#ifdef SFNUL_EXPORTS
		#define SFNUL_API __declspec( dllexport )
	#else
		#define SFNUL_API __declspec( dllimport )
	#endif
#else
	#define SFNUL_API
#endif

#ifdef _MSC_VER
	#pragma warning(disable : 4251) // Suppress a warning which is meaningless for us
	#pragma warning(disable : 4503) // Suppress warnings about truncated names. Enable again if linker errors occur.
#endif

#if !defined( NDEBUG )
	#define SFNUL_DEBUG
#endif

// Maximum bytes that SFNUL will queue before waiting for OS buffer to empty.
// If you think you need support for very large messages, go ahead and change this.
#define SFNUL_MAX_BUFFER_DATA_SIZE 65536

// Use asio in standalone header-only mode
#define ASIO_STANDALONE
#define ASIO_HEADER_ONLY

// Clang support, because asio doesn't detect it
#if defined( __clang__ )
	#if __has_feature( cxx_implicit_moves )
		#define ASIO_HAS_MOVE 1
	#endif

	#if __has_feature( cxx_variadic_templates )
		#define ASIO_HAS_VARIADIC_TEMPLATES 1
	#endif

	#if __has_feature( cxx_noexcept )
		#define ASIO_ERROR_CATEGORY_NOEXCEPT noexcept( true )
	#endif

	#define ASIO_HAS_HANDLER_HOOKS 1

	// We expect whoever uses clang to also have "proper" C++11 library support
	#define ASIO_HAS_STD_SYSTEM_ERROR 1
	#define ASIO_HAS_STD_ARRAY 1
	#define ASIO_HAS_STD_SHARED_PTR 1
	#define ASIO_HAS_STD_ATOMIC 1
	#define ASIO_HAS_STD_CHRONO 1
	#define ASIO_HAS_STD_ADDRESSOF 1
	#define ASIO_HAS_STD_FUNCTION 1
	#define ASIO_HAS_STD_TYPE_TRAITS 1
	#define ASIO_HAS_CSTDINT 1
#endif

namespace sfn {

typedef signed char Int8;
typedef unsigned char Uint8;

typedef signed short Int16;
typedef unsigned short Uint16;

typedef signed int Int32;
typedef unsigned int Uint32;

#if defined( _MSC_VER )

typedef signed __int64 Int64;
typedef unsigned __int64 Uint64;

#else

typedef signed long long Int64;
typedef unsigned long long Uint64;

#endif

}
