/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <memory>
#include <SFML/Network.hpp>
#include <SFNUL/Config.hpp>
#include <asio/ip/tcp.hpp>
#include <SFNUL/Socket.hpp>
#include <SFNUL/ReliableTransport.hpp>

namespace sfn {

class Endpoint;

#if defined( __GNUG__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#endif

/** TCP socket class.
 */
class SFNUL_API TcpSocket : public Socket, public ReliableTransport, public std::enable_shared_from_this<TcpSocket> {

#if defined( __GNUG__ )
#pragma GCC diagnostic pop
#endif

public:
	typedef std::shared_ptr<TcpSocket> Ptr; //!< Shared pointer.

	/** Create a TCP socket.
	 * @return A shared pointer that owns a new TCP socket.
	 */
	static Ptr Create();

	/** Destructor.
	 */
	~TcpSocket();

	/** Asynchronously connect this TCP socket to a remote endpoint.
	 * @param endpoint Remote endpoint.
	 */
	virtual void Connect( const Endpoint& endpoint ) override;

	/** Shutdown the socket for sending. This is required for graceful connection termination.
	 */
	virtual void Shutdown() override;

	/** Check if the local system has shut the socket down for sending.
	 * @return true if the local system has shut the socket down for sending.
	 */
	virtual bool LocalHasShutdown() const override;

	/** Check if the remote system has shut the socket down for sending.
	 * @return true if the remote system has shut the socket down for sending.
	 */
	virtual bool RemoteHasShutdown() const override;

	/** Check if the socket is part of an established TCP connection.
	 * @return true if the socket is part of an established TCP connection.
	 */
	virtual bool IsConnected() const override;

	/** Close the socket. This frees up the operating system resources assigned to the socket.
	 */
	virtual void Close() override;

	/// @cond
	virtual void Reset() override;
	/// @endcond

	/** Get the local endpoint of the established TCP connection this socket is part of.
	 * @return Local endpoint of the established TCP connection this socket is part of.
	 */
	virtual Endpoint GetLocalEndpoint() const override;

	/** Get the remote endpoint of the established TCP connection this socket is part of.
	 * @return Remote endpoint of the established TCP connection this socket is part of.
	 */
	virtual Endpoint GetRemoteEndpoint() const override;

	/** Queue data up for asynchronous sending over the established TCP connection this socket is part of.
	 * @param data Pointer to a block of memory containing the data to queue.
	 * @param size Size of the block of memory containing the data to queue.
	 */
	virtual void Send( const void* data, std::size_t size ) override;

	/** Dequeue data that was asynchronously received over the established TCP connection this socket is part of.
	 * @param data Pointer to a block of memory that will contain the data to dequeue.
	 * @param size Size of the block of memory that will contain the data to dequeue.
	 * @return Number of bytes of data that was actually dequeued.
	 */
	virtual std::size_t Receive( void* data, std::size_t size ) override;

	/** Queue an sf::Packet up for asynchronous sending over the established TCP connection this socket is part of.
	 * @param packet sf::Packet to queue.
	 */
	virtual void Send( sf::Packet& packet ) override;

	/** Dequeue an sf::Packet that was asynchronously received over the established TCP connection this socket is part of.
	 * @param packet sf::Packet to dequeue into.
	 * @return Size of the sf::Packet that was dequeued. This includes the size field of the packet. If no packet could be dequeued, this method will return 0.
	 */
	virtual std::size_t Receive( sf::Packet& packet ) override;

	/** Clear the send and receive queues of this socket.
	 */
	void ClearBuffers();

	/** Get the number of bytes queued for sending.
	 * @return Number of bytes queued for sending.
	 */
	std::size_t BytesToSend() const;

	/** Get the number of bytes queued for receiving.
	 * @return Number of bytes queued for receiving.
	 */
	std::size_t BytesToReceive() const;

	/** Set/Get the send queue warning threshold.
	 * @param limit Threshold above which to warn the user of the send queue size.
	 * @return Threshold above which to warn the user of the send queue size.
	 */
	std::size_t SendSoftLimit( std::size_t limit = 0 );

	/** Set/Get the send queue drop threshold.
	 * @param limit Threshold above which to drop new data to be queued.
	 * @return Threshold above which to drop new data to be queued.
	 */
	std::size_t SendHardLimit( std::size_t limit = 0 );

	/** Set/Get the receive queue warning threshold.
	 * @param limit Threshold above which to warn the user of the receive queue size.
	 * @return Threshold above which to warn the user of the receive queue size.
	 */
	std::size_t ReceiveSoftLimit( std::size_t limit = 0 );

	/** Set/Get the receive queue drop threshold.
	 * @param limit Threshold above which to drop new data to be queued.
	 * @return Threshold above which to drop new data to be queued.
	 */
	std::size_t ReceiveHardLimit( std::size_t limit = 0 );

	/** Get the seconds a socket should linger after it has been closed.
	 * @return Seconds a socket should linger after it has been closed. 0 means lingering is disabled.
	 */
	int GetLinger() const;

	/** Set the seconds a socket should linger after it has been closed.
	 * @param timeout Seconds a socket should linger after it has been closed. 0 disables lingering.
	 */
	void SetLinger( int timeout );

	/** Get whether TCP keep-alives are enabled.
	 * @return true if TCP keep-alives are enabled.
	 */
	bool GetKeepAlive() const;

	/** Set whether TCP keep-alives are enabled.
	 * @param keep_alive true to enable TCP keep-alives.
	 */
	void SetKeepAlive( bool keep_alive );

protected:
	/** Constructor.
	 */
	TcpSocket();

	/// @cond
	virtual void SetInternalSocket( void* internal_socket ) override;
	/// @endcond

private:
	void SendHandler( const asio::error_code& error, std::size_t bytes_sent );
	void ReceiveHandler( const asio::error_code& error, std::size_t bytes_received );

	asio::ip::tcp::socket m_socket;

	std::vector<char> m_send_buffer = {};
	std::vector<char> m_receive_buffer = {};

	std::array<char, 2048> m_send_memory;
	std::array<char, 2048> m_receive_memory;

	std::size_t m_send_limit_soft = 65536;
	std::size_t m_send_limit_hard = 65536 * 2;

	std::size_t m_receive_limit_soft = 65536;
	std::size_t m_receive_limit_hard = 65536 * 2;

	bool m_connected = false;
	bool m_request_shutdown = false;
	bool m_fin_sent = false;
	bool m_fin_received = false;

	bool m_receiving = false;
	bool m_sending = false;

	friend class TcpListener;
};

}
