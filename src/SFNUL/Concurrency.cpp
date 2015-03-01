/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <SFNUL/Concurrency.hpp>
#include <SFNUL/ConfigInternal.hpp>
#include <SFNUL/MakeUnique.hpp>

#if defined( SFNUL_USE_STD_THREAD )
	#include <thread>
	#include <mutex>
#elif defined( SFNUL_WIN32_THREADS )
	#include <windows.h>
	#include <process.h>
#elif defined( SFNUL_PTHREADS )
	#include <pthread.h>
#else
	#error No supported threading facility available. Review CMake configuration for more information.
#endif

/// @cond
namespace sfn {

#if defined( __GNUG__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

#if !defined( SFNUL_USE_STD_THREAD )
	#if defined( SFNUL_WIN32_THREADS )
	unsigned __stdcall run( void* data );
	#elif defined( SFNUL_PTHREADS )
	void* run( void* data );
	#endif
#endif

class ThreadImpl {
public:
#if defined( SFNUL_USE_STD_THREAD )
	ThreadImpl( std::function<void()> thread_function ) :
		thread{ thread_function }
	{
	}

	~ThreadImpl() {
		thread.join();
	}

	mutable std::thread thread;
#elif defined( SFNUL_WIN32_THREADS )
	ThreadImpl( std::function<void()> thread_function ) :
		function{ thread_function }
	{
		thread = reinterpret_cast<HANDLE>( _beginthreadex( nullptr,	0, &run, this, 0, nullptr	) );
	}

	~ThreadImpl() {
		if( thread ) {
			WaitForSingleObject( thread, INFINITE );
			CloseHandle( thread );
		}
	}

	mutable HANDLE thread{};
#elif defined( SFNUL_PTHREADS )
	ThreadImpl( std::function<void()> thread_function ) :
		function{ thread_function }
	{
		pthread_create( &thread, nullptr, &run, this );
	}

	~ThreadImpl() {
		pthread_join( thread, nullptr );
	}

	mutable pthread_t thread{};
#endif

	std::function<void()> function;
};

#if !defined( SFNUL_USE_STD_THREAD )
	#if defined( SFNUL_WIN32_THREADS )
	unsigned __stdcall run( void* data ) {
		static_cast<sfn::ThreadImpl*>( data )->function();
		_endthread();
		return 0;
	}
	#elif defined( SFNUL_PTHREADS )
	void* run( void* data ) {
		static_cast<sfn::ThreadImpl*>( data )->function();
		return nullptr;
	}
	#endif
#endif

Thread::Thread( std::function<void()> thread_function ) :
	m_impl{ make_unique<ThreadImpl>( thread_function ) }
{
}

Thread::~Thread() = default;

class Atomic::AtomicImpl {
public:
#if defined( SFNUL_USE_STD_THREAD )
	void Lock() const {
		mutex.lock();
	}

	void Unlock() const {
		mutex.unlock();
	}

	mutable std::recursive_mutex mutex;
#elif defined( SFNUL_WIN32_THREADS )
	AtomicImpl() {
		InitializeCriticalSection( &mutex );
	}

	~AtomicImpl() {
		DeleteCriticalSection( &mutex );
	}

	void Lock() const {
		EnterCriticalSection( &mutex );
	}

	void Unlock() const {
		LeaveCriticalSection( &mutex );
	}

	mutable CRITICAL_SECTION mutex;
#elif defined( SFNUL_PTHREADS )
	AtomicImpl() {
		pthread_mutexattr_t mutex_attr;
		pthread_mutexattr_init( &mutex_attr );
		pthread_mutexattr_settype( &mutex_attr, PTHREAD_MUTEX_RECURSIVE );
		pthread_mutex_init( &mutex, &mutex_attr );
		pthread_mutexattr_destroy( &mutex_attr );
	}

	~AtomicImpl() {
		pthread_mutex_destroy( &mutex );
	}

	void Lock() const {
		pthread_mutex_lock( &mutex );
	}

	void Unlock() const {
		pthread_mutex_unlock( &mutex );
	}

	mutable pthread_mutex_t mutex;
#endif
};

Atomic::Atomic() :
	m_impl{ make_unique<AtomicImpl>() }
{
}

Atomic::~Atomic() = default;

void Atomic::LockMutex() const {
	m_impl->Lock();
}

void Atomic::UnlockMutex() const {
	m_impl->Unlock();
}

Atomic::ScopedLock::ScopedLock( Atomic* atomic ) :
	m_atomic{ atomic }
{
	m_atomic->LockMutex();
}

Atomic::ScopedLock::~ScopedLock() {
	m_atomic->UnlockMutex();
}

Atomic::ScopedLock Atomic::AcquireLock() const {
	return ScopedLock{ const_cast<Atomic*>( this ) };
}

#if defined( __GNUG__ )
#pragma GCC diagnostic pop
#endif

}
/// @endcond
