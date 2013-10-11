/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <memory>
#include <deque>
#include <SFNUL/Config.hpp>
#include <asio/ip/tcp.hpp>
#include <SFNUL/Socket.hpp>

namespace sfn {

class Endpoint;
class TcpSocket;

#if defined( __GNUG__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#endif

/** TCP listener class. This class listens for incoming connections and asynchronously accepts them.
 */
class SFNUL_API TcpListener : public Socket, public std::enable_shared_from_this<TcpListener> {

#if defined( __GNUG__ )
#pragma GCC diagnostic pop
#endif

public:
	typedef std::shared_ptr<TcpListener> Ptr; //!< Shared pointer.

	/// @cond
	static const int default_backlog = asio::ip::tcp::acceptor::max_connections;
	/// @endcond

	/** Create a TCP listener.
	 * @return A shared pointer that owns a new TCP listener.
	 */
	static Ptr Create();

	/** Destructor.
	 */
	~TcpListener();

	/** Listens on the endpoint for incoming TCP connections and asynchronously accepts them.
	 * @param endpoint Endpoint to listen on.
	 * @param backlog Maximum number of connections to allow outstanding in the TCP backlog. Default: SOMAXCONN
	 */
	void Listen( const Endpoint& endpoint, int backlog = default_backlog );

	/** Close the socket on which the TCP listener is listening.
	 */
	void Close();

	/** Get the endpoint on which the TCP listener is listening.
	 * @return Endpoint on which the TCP listener is listening.
	 */
	Endpoint GetEndpoint() const;

	/** Gets the next pending connection in queue.
	 * @return TcpSocket::Ptr of the next pending connection in queue.
	 */
	template<class T = TcpSocket>
	std::shared_ptr<T> GetPendingConnection();

	/** Sets/Gets warning threshold above which the user is warned that there is a high number of pending connections.
	 * @param limit New threshold value. Default: 0 which means don't change
	 * @return Warning threshold value.
	 */
	std::size_t ConnectionSoftLimit( std::size_t limit = 0 );

	/** Sets/Gets threshold above which incoming connection requests are dropped.
	 * @param limit New threshold value. Default: 0 which means don't change
	 * @return Warning threshold value.
	 */
	std::size_t ConnectionHardLimit( std::size_t limit = 0 );

protected:
	/** Constructor.
	 */
	TcpListener();

private:
	void AcceptHandler( const asio::error_code& error, std::shared_ptr<asio::ip::tcp::socket> socket );

	asio::ip::tcp::acceptor m_acceptor;

	std::deque<asio::ip::tcp::socket> m_new_connections = {};

	std::size_t m_connection_limit_soft = 1024;
	std::size_t m_connection_limit_hard = 1024 * 2;

	bool m_listening = false;
};

}

#include <SFNUL/TcpListener.inl>
