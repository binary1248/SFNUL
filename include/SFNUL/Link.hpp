/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <SFNUL/Config.hpp>
#include <SFNUL/ReliableTransport.hpp>
#include <SFNUL/DataTypes.hpp>
#include <memory>

namespace sfn {

class Message;

class SFNUL_API LinkBase : public ReliableTransport {
public:
	typedef Uint8 stream_id_type;

	/** Asynchronously connect this link to a remote endpoint.
	 * @param endpoint Remote endpoint.
	 */
	virtual void Connect( const Endpoint& endpoint ) override;

	/** Shutdown the link for sending. This is required for graceful connection termination.
	 */
	virtual void Shutdown() override;

	/** Check if the local system has shut the link down for sending.
	 * @return true if the local system has shut the link down for sending.
	 */
	virtual bool LocalHasShutdown() const override;

	/** Check if the remote system has shut the link down for sending.
	 * @return true if the remote system has shut the link down for sending.
	 */
	virtual bool RemoteHasShutdown() const override;

	/** Check if the link is part of an established connection.
	 * @return true if the link is part of an established connection.
	 */
	virtual bool IsConnected() const override;

	/** Close the link. This frees up the operating system resources assigned to the link.
	 */
	virtual void Close() override;

	/** Get the local endpoint of the established connection this link is part of.
	 * @return Local endpoint of the established connection this link is part of.
	 */
	virtual Endpoint GetLocalEndpoint() const override;

	/** Get the remote endpoint of the established connection this link is part of.
	 * @return Remote endpoint of the established connection this link is part of.
	 */
	virtual Endpoint GetRemoteEndpoint() const override;

	/** Queue data up for asynchronous sending over the established connection this link is part of.
	 * @param data Pointer to a block of memory containing the data to queue.
	 * @param size Size of the block of memory containing the data to queue.
	 * @return true if the data could be queued. If false is returned, retry again later.
	 */
	virtual bool Send( const void* data, std::size_t size ) override;

	/** Queue data up for asynchronous sending over the established connection this link is part of.
	 * @param stream_id Identifier of the stream to send on.
	 * @param data Pointer to a block of memory containing the data to queue.
	 * @param size Size of the block of memory containing the data to queue.
	 * @return true if the data could be queued. If false is returned, retry again later.
	 */
	bool Send( stream_id_type stream_id, const void* data, std::size_t size );

	/** Dequeue data that was asynchronously received over the established connection this link is part of.
	 * @param data Pointer to a block of memory that will contain the data to dequeue.
	 * @param size Size of the block of memory that will contain the data to dequeue.
	 * @return Number of bytes of data that was actually dequeued.
	 */
	virtual std::size_t Receive( void* data, std::size_t size ) override;

	/** Dequeue data on a stream that was asynchronously received over the established connection this link is part of.
	 * @param stream_id Identifier of the stream to dequeue from.
	 * @param data Pointer to a block of memory that will contain the data to dequeue.
	 * @param size Size of the block of memory that will contain the data to dequeue.
	 * @return Number of bytes of data that was actually dequeued.
	 */
	std::size_t Receive( stream_id_type stream_id, void* data, std::size_t size );

	/** Queue a Message up for asynchronous sending over the established connection this link is part of.
	 * @param message Message to queue.
	 * @return true if the message could be queued. If false is returned, retry again later.
	 */
	virtual bool Send( const Message& message ) override;

	/** Queue a Message up for asynchronous sending over the established connection this link is part of.
	 * @param stream_id Identifier of the stream to send on.
	 * @param message Message to queue.
	 * @return true if the message could be queued. If false is returned, retry again later.
	 */
	bool Send( stream_id_type stream_id, const Message& message );

	/** Dequeue an Message that was asynchronously received over the established connection this link is part of.
	 * @param message Message to dequeue into.
	 * @return Size of the Message that was dequeued. This includes the size field of the Message. If no Message could be dequeued, this method will return 0.
	 */
	virtual std::size_t Receive( Message& message ) override;

	/** Dequeue an Message that was asynchronously received over the established connection this link is part of.
	 * @param stream_id Identifier of the stream to dequeue from.
	 * @param message Message to dequeue into.
	 * @return Size of the Message that was dequeued. This includes the size field of the Message. If no Message could be dequeued, this method will return 0.
	 */
	std::size_t Receive( stream_id_type stream_id, Message& message );

	/** Clear the send and receive queues of this transport.
	 */
	virtual void ClearBuffers() override;

	/** Get the number of bytes queued for sending.
	 * @return Number of bytes queued for sending.
	 */
	virtual std::size_t BytesToSend() const override;

	/** Get the number of bytes queued for receiving.
	 * @return Number of bytes queued for receiving.
	 */
	virtual std::size_t BytesToReceive() const override;

protected:
	/// @cond
	LinkBase() = default;
	~LinkBase() = default;

	virtual void SetInternalSocket( void* internal_socket ) override;

	virtual ReliableTransport* GetInternalTransport() = 0;
	virtual const ReliableTransport* GetInternalTransport() const = 0;

	bool m_segment_active{ false };
	/// @endcond

private:
	typedef Uint32 segment_size_type;

	stream_id_type m_current_stream_reader = 0;
	segment_size_type m_segment_remaining = 0;
};

template<typename T>
class SFNUL_API Link : public LinkBase {
	static_assert( std::is_base_of<ReliableTransport, T>::value, "Links can only be set up over reliable transports." );

public:
	/** Constructor.
	 */
	Link();

	/** Constructor.
	 * @param transport Underlying transport.
	 */
	Link( std::shared_ptr<T> transport );

	void SetTransport( std::shared_ptr<T> transport );

	std::shared_ptr<T> GetTransport();
	std::shared_ptr<const T> GetTransport() const;

protected:

	virtual ReliableTransport* GetInternalTransport() override;
	virtual const ReliableTransport* GetInternalTransport() const override;

private:
	std::shared_ptr<T> m_transport{};
};

}

#include <SFNUL/Link.inl>
