/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <memory>
#include <map>
#include <SFNUL/Config.hpp>
#include <asio/ip/udp.hpp>
#include <SFNUL/Socket.hpp>
#include <SFNUL/Transport.hpp>

namespace sfn {

class Endpoint;

#if defined( __GNUG__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#endif

/** UDP socket class.
 */
class SFNUL_API UdpSocket : public Socket, public Transport, public std::enable_shared_from_this<UdpSocket> {

#if defined( __GNUG__ )
#pragma GCC diagnostic pop
#endif

public:
	typedef std::shared_ptr<UdpSocket> Ptr; //!< Shared pointer.

	/** Create a UDP socket.
	 * @return A shared pointer that owns a new UDP socket.
	 */
	static Ptr Create();

	/** Destructor.
	 */
	~UdpSocket();

	/** Bind this UDP socket to a local endpoint.
	 * @param endpoint Local endpoint to bind the UDP socket to.
	 */
	void Bind( const Endpoint& endpoint );

	/** Close the socket. This frees up the operating system resources assigned to the socket.
	 */
	void Close();

	/** Get the local endpoint this UDP socket is bound to.
	 * @return Local endpoint this UDP socket is bound to.
	 */
	Endpoint GetLocalEndpoint() const;

	/** Queue data for asynchronous sending to a remote endpoint over this UDP socket.
	 * @param data Pointer to a block of memory containing the data to queue.
	 * @param size Size of the block of memory containing the data to queue.
	 * @param endpoint Remote endpoint to send to.
	 */
	void SendTo( const void* data, std::size_t size, const Endpoint& endpoint );

	/** Dequeue data that was asynchronously received from a remote endpoint over this UDP socket.
	 * @param data Pointer to a block of memory that will contain the data to dequeue.
	 * @param size Size of the block of memory that will contain the data to dequeue.
	 * @param endpoint Remote endpoint to receive from.
	 * @return Number of bytes of data that was actually dequeued.
	 */
	std::size_t ReceiveFrom( void* data, std::size_t size, const Endpoint& endpoint );

	/** Clear the send and receive queues of this socket.
	 */
	void ClearBuffers();

	/** Get the number of bytes to be dequeued for a specific remote endpoint.
	 * @param endpoint Remote endpoint.
	 * @return Number of bytes to be dequeued.
	 */
	std::size_t BytesToReceive( const Endpoint& endpoint ) const;

	/** Get the remote endpoints with data to be dequeued.
	 * @return std::deque<Endpoint> containing the remote endpoints with data to be dequeued.
	 */
	std::deque<Endpoint> PendingEndpoints() const;

protected:
	/** Constructor.
	 */
	UdpSocket();

	/// @cond
	virtual void SetInternalSocket( void* internal_socket ) override;
	/// @endcond

private:
	void SendHandler( const asio::error_code& error, std::size_t bytes_sent, asio::ip::udp::endpoint endpoint, std::shared_ptr<std::vector<char>> buffer );
	void ReceiveHandler( const asio::error_code& error, std::size_t bytes_received, std::shared_ptr<asio::ip::udp::endpoint> endpoint_ptr );

	asio::ip::udp::socket m_socket;

	std::map<asio::ip::udp::endpoint, std::vector<char>> m_receive_buffer = {};

	std::array<char, 2048> m_receive_memory;

	std::size_t m_pending_data = 0;

	bool m_receiving = false;
};

}
