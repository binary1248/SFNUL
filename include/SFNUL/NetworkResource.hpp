#pragma once

#include <memory>
#include <SFML/System.hpp>
#include <asio/io_service.hpp>
#include <asio/strand.hpp>
#include <SFNUL/Config.hpp>
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
void Start( std::size_t threads = 1 );

/** Stops and waits for all network threads to end.
 */
void Stop();

}
