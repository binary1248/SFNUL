/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <SFNUL/Config.hpp>

#if defined( _WIN32 ) || defined( __WIN32__ )
	#define SFNUL_SYSTEM_WINDOWS
	#define WIN32_LEAN_AND_MEAN

	#ifndef NOMINMAX
		#define NOMINMAX
	#endif

	#if !defined( WIN32 )
		#define WIN32
	#endif

	#if !defined( _WIN32_WINNT )
		#define _WIN32_WINNT 0x0501
	#endif
#endif

#if !defined( NDEBUG )
	#define SFNUL_DEBUG
#endif

// Use asio in standalone header-only mode
#define ASIO_STANDALONE
#define ASIO_HEADER_ONLY

// Use Win32 threads on Windows and pthreads elsewhere unless explicitly requested.
#if !defined( SFNUL_USE_STD_THREAD ) && !defined( SFNUL_WIN32_THREADS ) && !defined( SFNUL_PTHREADS )
	#if defined( SFNUL_SYSTEM_WINDOWS )
		#define SFNUL_WIN32_THREADS
	#else
		#define SFNUL_PTHREADS
	#endif
#endif

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

// VS2013 support, because asio doesn't detect it
#if defined( _MSC_VER ) && ( _MSC_VER == 1800 )
	#define ASIO_HAS_MOVE 1
	#define ASIO_HAS_VARIADIC_TEMPLATES 1
	#define ASIO_HAS_HANDLER_HOOKS 1
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

// std::min and std::max in asio
#include <algorithm>
