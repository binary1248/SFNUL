/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <SFML/Network.hpp>
#include <SFNUL/Config.hpp>
#include <SFNUL/Transport.hpp>

namespace sfn {

class Endpoint;

/** Reliable Transport interface.
 */
class SFNUL_API ReliableTransport : public Transport {

public:

	/** Destructor.
	 */
	virtual ~ReliableTransport();

	/** Asynchronously connect this socket to a remote endpoint.
	 * @param endpoint Remote endpoint.
	 */
	virtual void Connect( const Endpoint& endpoint ) = 0;

	/** Shutdown the socket for sending. This is required for graceful connection termination.
	 */
	virtual void Shutdown() = 0;

	/** Check if the local system has shut the socket down for sending.
	 * @return true if the local system has shut the socket down for sending.
	 */
	virtual bool LocalHasShutdown() const = 0;

	/** Check if the remote system has shut the socket down for sending.
	 * @return true if the remote system has shut the socket down for sending.
	 */
	virtual bool RemoteHasShutdown() const = 0;

	/** Check if the socket is part of an established connection.
	 * @return true if the socket is part of an established connection.
	 */
	virtual bool IsConnected() const = 0;

	/** Close the socket. This frees up the operating system resources assigned to the socket.
	 */
	virtual void Close() = 0;

	/** Get the local endpoint of the established connection this socket is part of.
	 * @return Local endpoint of the established connection this socket is part of.
	 */
	virtual Endpoint GetLocalEndpoint() const = 0;

	/** Get the remote endpoint of the established connection this socket is part of.
	 * @return Remote endpoint of the established connection this socket is part of.
	 */
	virtual Endpoint GetRemoteEndpoint() const = 0;

	/** Queue data up for asynchronous sending over the established connection this socket is part of.
	 * @param data Pointer to a block of memory containing the data to queue.
	 * @param size Size of the block of memory containing the data to queue.
	 */
	virtual void Send( const void* data, std::size_t size ) = 0;

	/** Dequeue data that was asynchronously received over the established connection this socket is part of.
	 * @param data Pointer to a block of memory that will contain the data to dequeue.
	 * @param size Size of the block of memory that will contain the data to dequeue.
	 * @return Number of bytes of data that was actually dequeued.
	 */
	virtual std::size_t Receive( void* data, std::size_t size ) = 0;

	/** Queue an sf::Packet up for asynchronous sending over the established connection this socket is part of.
	 * @param packet sf::Packet to queue.
	 */
	virtual void Send( sf::Packet& packet ) = 0;

	/** Dequeue an sf::Packet that was asynchronously received over the established connection this socket is part of.
	 * @param packet sf::Packet to dequeue into.
	 * @return Size of the sf::Packet that was dequeued. This includes the size field of the packet. If no packet could be dequeued, this method will return 0.
	 */
	virtual std::size_t Receive( sf::Packet& packet ) = 0;

protected:

	/** Ctor.
	 */
	ReliableTransport();

	/** Used to inform subclasses that the transport has connected.
	 */
	virtual void OnConnected();

	/** Used to inform subclasses that the transport has disconnected.
	 */
	virtual void OnDisconnected();

private:

};

}
