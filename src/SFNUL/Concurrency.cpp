/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <SFNUL/Concurrency.hpp>

/// @cond
namespace sfn {

Thread::Thread( std::function<void()> thread_function ) :
	m_function{ thread_function }
{
#if defined( SFNUL_WIN32_THREADS )
	m_thread = reinterpret_cast<HANDLE>( _beginthreadex( nullptr,	0, &Thread::run, this, 0, nullptr	) );
#elif defined( SFNUL_PTHREADS )
	pthread_create( &m_thread, nullptr, &Thread::run, this );
#endif
}

Thread::~Thread() {
#if defined( SFNUL_WIN32_THREADS )
	if( m_thread ) {
		WaitForSingleObject( m_thread, INFINITE );
		CloseHandle( m_thread );
	}
#elif defined( SFNUL_PTHREADS )
	pthread_join( m_thread, nullptr );
#endif
}

#if defined( SFNUL_WIN32_THREADS )
unsigned __stdcall Thread::run( void* data ) {
	static_cast<Thread*>( data )->m_function();
	_endthread();
	return 0;
}
#elif defined( SFNUL_PTHREADS )
void* Thread::run( void* data ) {
	static_cast<Thread*>( data )->m_function();
	return nullptr;
}
#endif

Atomic::Atomic() {
#if defined( SFNUL_WIN32_THREADS )
	InitializeCriticalSection( &m_mutex );
#elif defined( SFNUL_PTHREADS )
	pthread_mutexattr_t mutex_attr;
	pthread_mutexattr_init( &mutex_attr );
	pthread_mutexattr_settype( &mutex_attr, PTHREAD_MUTEX_RECURSIVE );
	pthread_mutex_init( &m_mutex, &mutex_attr );
	pthread_mutexattr_destroy( &mutex_attr );
#endif
}

Atomic::~Atomic() {
#if defined( SFNUL_WIN32_THREADS )
	DeleteCriticalSection( &m_mutex );
#elif defined( SFNUL_PTHREADS )
	pthread_mutex_destroy( &m_mutex );
#endif
}

void Atomic::LockMutex() const {
#if defined( SFNUL_WIN32_THREADS )
	EnterCriticalSection( &m_mutex );
#elif defined( SFNUL_PTHREADS )
	pthread_mutex_lock( &m_mutex );
#endif
}

void Atomic::UnlockMutex() const {
#if defined( SFNUL_WIN32_THREADS )
	LeaveCriticalSection( &m_mutex );
#elif defined( SFNUL_PTHREADS )
	pthread_mutex_unlock( &m_mutex );
#endif
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

}
/// @endcond
