/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <functional>
#include <SFNUL/Config.hpp>

#if defined( SFNUL_WIN32_THREADS )
#include <windows.h>
#include <process.h>
#elif defined( SFNUL_PTHREADS )
#include <pthread.h>
#else
#error No supported threading facility available. Review CMake configuration for more information.
#endif

/// @cond
namespace sfn {

class SFNUL_API Thread {
public:
	Thread( std::function<void()> thread_function );
	~Thread();

	Thread( const Thread& other ) = delete;

#if !defined( _MSC_VER )
	Thread( Thread&& other ) = default;
#endif

	Thread& operator=( const Thread& other ) = delete;
	Thread& operator=( Thread&& other ) = delete;
private:
#if defined( __GNUG__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

#if defined( SFNUL_WIN32_THREADS )
	static unsigned __stdcall run( void* data );
	mutable HANDLE m_thread{};
#elif defined( SFNUL_PTHREADS )
	static void* run( void* data );
	mutable pthread_t m_thread{};
#endif

#if defined( __GNUG__ )
#pragma GCC diagnostic pop
#endif

	std::function<void()> m_function;
};

class SFNUL_API Atomic {
protected:
	class SFNUL_API ScopedLock {
	public:
		ScopedLock( Atomic* atomic );
		~ScopedLock();
		ScopedLock( const ScopedLock& other ) = delete;

#if !defined( _MSC_VER )
		ScopedLock( ScopedLock&& other ) = default;
#endif

		ScopedLock& operator=( const ScopedLock& other ) = delete;
		ScopedLock& operator=( ScopedLock&& other ) = delete;
	private:
		Atomic* m_atomic;
	};

	Atomic();
	~Atomic();
	Atomic( const Atomic& other ) = delete;

#if !defined( _MSC_VER )
	Atomic( Atomic&& other ) = default;
#endif

	Atomic& operator=( const Atomic& other ) = delete;

#if !defined( _MSC_VER )
	Atomic& operator=( Atomic&& other ) = default;
#endif

	ScopedLock AcquireLock() const;
private:
	void LockMutex() const;
	void UnlockMutex() const;
#if defined( __GNUG__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

#if defined( SFNUL_WIN32_THREADS )
	mutable CRITICAL_SECTION m_mutex;
#elif defined( SFNUL_PTHREADS )
	mutable pthread_mutex_t m_mutex;
#endif

#if defined( __GNUG__ )
#pragma GCC diagnostic pop
#endif
};

}
/// @endcond
