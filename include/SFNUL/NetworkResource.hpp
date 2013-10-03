/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <memory>
#include <SFML/System.hpp>
#include <SFNUL/Config.hpp>
#include <asio/io_service.hpp>
#include <asio/strand.hpp>
#include <SFNUL/NonCopyable.hpp>

namespace sfn {

class IpAddress;

/** Network resource base class.
 */
class SFNUL_API NetworkResource : public NonCopyable {
public:
	/** Constructor.
	 */
	NetworkResource();

	/** Destructor.
	 */
	~NetworkResource();

private:
	std::shared_ptr<asio::io_service> m_io_service;

protected:
	/** Get the associated asio io_service.
	 * @return associated asio io_service.
	 */
	asio::io_service& GetIOService() const;

	/// @cond
	mutable asio::strand m_strand;
	mutable sf::Mutex m_mutex;
	/// @endcond

private:
	static std::weak_ptr<asio::io_service> m_shared_io_service;

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
