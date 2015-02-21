/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <SFNUL/Config.hpp>
#include <SFNUL/Socket.hpp>
#include <memory>
#include <deque>

namespace sfn {

class Endpoint;
class TcpSocket;

/** TCP listener class. This class listens for incoming connections and asynchronously accepts them.
 */
class SFNUL_API TcpListener : public Socket, public std::enable_shared_from_this<TcpListener> {

public:
	typedef std::shared_ptr<TcpListener> Ptr; //!< Shared pointer.

	/// @cond
	static const int default_backlog = 128;
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

	/** Check if there are any pending connections.
	 * @return true if there are any pending connections.
	 */
	bool HasPendingConnections() const;

	/** Gets the next pending connection in queue.
	 * @return TcpSocket::Ptr of the next pending connection in queue.
	 */
	template<class T = TcpSocket>
	std::shared_ptr<T> GetPendingConnection();

protected:
	/** Constructor.
	 */
	TcpListener();

private:
	void* GetFirstPendingConnection();
	void RemoveFirstPendingConnection();

	class TcpListenerImpl;
	std::unique_ptr<TcpListenerImpl> m_impl;
};

}

#include <SFNUL/TcpListener.inl>
