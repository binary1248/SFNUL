/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <cstddef>
#include <SFNUL/Config.hpp>
#include <SFNUL/Transport.hpp>

namespace sfn {

class Message;

/** Reliable Transport interface.
 */
class SFNUL_API ReliableTransport : public Transport {

public:

	/** Destructor.
	 */
	virtual ~ReliableTransport();

	/** Asynchronously connect this transport to a remote endpoint.
	 * @param endpoint Remote endpoint.
	 */
	virtual void Connect( const Endpoint& endpoint ) = 0;

	/** Shutdown the transport for sending. This is required for graceful connection termination.
	 */
	virtual void Shutdown() = 0;

	/** Check if the local system has shut the transport down for sending.
	 * @return true if the local system has shut the transport down for sending.
	 */
	virtual bool LocalHasShutdown() const = 0;

	/** Check if the remote system has shut the transport down for sending.
	 * @return true if the remote system has shut the transport down for sending.
	 */
	virtual bool RemoteHasShutdown() const = 0;

	/** Check if the transport is part of an established connection.
	 * @return true if the transport is part of an established connection.
	 */
	virtual bool IsConnected() const = 0;

	/** Close the transport. This frees up the operating system resources assigned to the transport.
	 */
	virtual void Close() = 0;

	/// @cond
	virtual void Reset();
	/// @endcond

	/** Get the remote endpoint of the established connection this transport is part of.
	 * @return Remote endpoint of the established connection this transport is part of.
	 */
	virtual Endpoint GetRemoteEndpoint() const = 0;

	/** Queue data up for asynchronous sending over the established connection this transport is part of.
	 * @param data Pointer to a block of memory containing the data to queue.
	 * @param size Size of the block of memory containing the data to queue.
	 * @return true if the data could be queued. If false is returned, retry again later.
	 */
	virtual bool Send( const void* data, std::size_t size ) = 0;

	/** Dequeue data that was asynchronously received over the established connection this transport is part of.
	 * @param data Pointer to a block of memory that will contain the data to dequeue.
	 * @param size Size of the block of memory that will contain the data to dequeue.
	 * @return Number of bytes of data that was actually dequeued.
	 */
	virtual std::size_t Receive( void* data, std::size_t size ) = 0;

	/** Queue a Message up for asynchronous sending over the established connection this transport is part of.
	 * @param message Message to queue.
	 * @return true if the message could be queued. If false is returned, retry again later.
	 */
	virtual bool Send( const Message& message ) = 0;

	/** Dequeue an Message that was asynchronously received over the established connection this transport is part of.
	 * @param message Message to dequeue into.
	 * @return Size of the Message that was dequeued. This includes the size field of the Message. If no Message could be dequeued, this method will return 0.
	 */
	virtual std::size_t Receive( Message& message ) = 0;

	/** Get the number of bytes queued for sending.
	 * @return Number of bytes queued for sending.
	 */
	virtual std::size_t BytesToSend() const = 0;

	/** Get the number of bytes queued for receiving.
	 * @return Number of bytes queued for receiving.
	 */
	virtual std::size_t BytesToReceive() const = 0;

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
