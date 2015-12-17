/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <SFNUL/Config.hpp>
#include <functional>
#include <memory>

/// @cond
namespace sfn {

class ThreadImpl;

class SFNUL_API Thread {
public:
	Thread( std::function<void()> thread_function );
	~Thread();

	Thread( const Thread& other ) = delete;

#if !defined( SFNUL_BROKEN_CXX11 )
	Thread( Thread&& other ) = default;
#endif

	Thread& operator=( const Thread& other ) = delete;
	Thread& operator=( Thread&& other ) = delete;
private:
	std::unique_ptr<ThreadImpl> m_impl;
};

class SFNUL_API Atomic {
protected:
	class SFNUL_API ScopedLock {
	public:
		ScopedLock( Atomic* atomic );
		~ScopedLock();
		ScopedLock( const ScopedLock& other ) = delete;

#if !defined( SFNUL_BROKEN_CXX11 )
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

#if !defined( SFNUL_BROKEN_CXX11 )
	Atomic( Atomic&& other ) = default;
#endif

	Atomic& operator=( const Atomic& other ) = delete;

#if !defined( SFNUL_BROKEN_CXX11 )
	Atomic& operator=( Atomic&& other ) = default;
#endif

	ScopedLock AcquireLock() const;
private:
	void LockMutex() const;
	void UnlockMutex() const;

	class AtomicImpl;
	std::unique_ptr<AtomicImpl> m_impl;
};

}
/// @endcond
