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

// Use asio in standalone header-only mode
#define ASIO_STANDALONE
#define ASIO_HEADER_ONLY

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
