/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <SFNUL/Config.hpp>
#include <SFNUL/Concurrency.hpp>
#include <memory>

namespace sfn {

class IpAddress;

/** Network resource base class.
 */
class SFNUL_API NetworkResource : protected Atomic {
protected:
	/** Constructor.
	 */
	NetworkResource();

	/** Destructor.
	 */
	~NetworkResource();

	NetworkResource( const NetworkResource& other ) = delete;

#if !defined( _MSC_VER )
	NetworkResource( NetworkResource&& other ) = default;
#endif

	NetworkResource& operator=( const NetworkResource& other ) = delete;

#if !defined( _MSC_VER )
	NetworkResource& operator=( NetworkResource&& other ) = default;
#endif

	/** Get the associated asio io_service.
	 * @return associated asio io_service.
	 */
	void* GetIOService() const;

	/** Get the associated asio strand.
	 * @return associated asio strand.
	 */
	void* GetStrand() const;

private:
	class NetworkResourceImpl;
	std::unique_ptr<NetworkResourceImpl> m_impl;

	friend void Start( std::size_t threads );
	friend void Stop();
	friend class IpAddress;
};

/** Starts n threads that handle asynchronous network IO.
 * @param threads Number of threads to start. Default: 1
 */
SFNUL_API void Start( std::size_t threads = 1 );

/** Stops and waits for all network threads to end.
 */
SFNUL_API void Stop();

}
