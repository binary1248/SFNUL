/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#define SFNUL_MAJOR_VERSION 0
#define SFNUL_MINOR_VERSION 3

#if defined( SFNUL_SYSTEM_WINDOWS ) && !defined( SFNUL_STATIC )
	#if defined( SFNUL_EXPORTS )
		#define SFNUL_API __declspec( dllexport )
	#else
		#define SFNUL_API __declspec( dllimport )
	#endif
#else
	#define SFNUL_API
#endif

#if defined( _MSC_VER )
	#pragma warning(disable : 4251) // Suppress a warning which is meaningless for us
	#pragma warning(disable : 4503) // Suppress warnings about truncated names. Enable again if linker errors occur.

	#if ( _MSC_VER < 1900 )
		#define SFNUL_BROKEN_CXX11
	#endif
#endif
