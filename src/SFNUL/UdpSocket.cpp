/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <functional>
#include <algorithm>
#include <iostream>
#include <SFNUL/Config.hpp>
#include <asio/buffer.hpp>
#include <SFNUL/Endpoint.hpp>
#include <SFNUL/UdpSocket.hpp>

namespace sfn {
namespace {
struct UdpSocketMaker : public UdpSocket {};
}

UdpSocket::UdpSocket() :
	m_socket{ GetIOService() },
	m_receive_memory{ {} }
{
}

UdpSocket::~UdpSocket() {
	Close();
}

UdpSocket::Ptr UdpSocket::Create() {
	return std::make_shared<UdpSocketMaker>();
}

void UdpSocket::Bind( const Endpoint& endpoint ) {
	auto lock = AcquireLock();

	auto asio_endpoint = endpoint.GetInternalEndpoint<asio::ip::udp>();

	if( !m_socket.is_open() ) {
		m_socket.open( asio_endpoint.protocol() );
	}

	asio::error_code error;

	m_socket.bind( asio_endpoint, error );

	if( error ) {
		std::cerr << "Bind() Error: " << error.message() << "\n";
		return;
	}

	ReceiveHandler( asio::error_code{}, 0, nullptr );
}

void UdpSocket::Close() {
	auto lock = AcquireLock();

	if( !m_socket.is_open() ) {
		return;
	}

	m_socket.close();
}

void UdpSocket::SendHandler( const asio::error_code& error, std::size_t bytes_sent, asio::ip::udp::endpoint endpoint, std::shared_ptr<std::vector<char>> buffer ) {
	{
		auto lock = AcquireLock();

		if( ( error == asio::error::operation_aborted ) || ( error == asio::error::connection_aborted ) || ( error == asio::error::connection_reset ) ) {
			return;
		}
		else if( error ) {
			std::cerr << "Async Send Error: " << error.message() << "\n";
			return;
		}

		if( bytes_sent == buffer->size() ) {
			return;
		}
		else {
			auto new_buffer = std::make_shared<std::vector<char>>( buffer->size() - bytes_sent );

			std::copy_n( buffer->begin() + static_cast<int>( bytes_sent ), new_buffer->size(), new_buffer->begin() );

			m_socket.async_send_to( asio::buffer( *new_buffer ), endpoint,
				m_strand.wrap(
					std::bind(
						[]( std::weak_ptr<UdpSocket> socket, const asio::error_code& handler_error, std::size_t handler_bytes_sent, asio::ip::udp::endpoint handler_endpoint, std::shared_ptr<std::vector<char>> handler_buffer ) {
							auto shared_socket = socket.lock();

							if( !shared_socket ) {
								return;
							}

							auto handler_lock = shared_socket->AcquireLock();
							shared_socket->SendHandler( handler_error, handler_bytes_sent, handler_endpoint, handler_buffer );
						},
						std::weak_ptr<UdpSocket>( shared_from_this() ), std::placeholders::_1, std::placeholders::_2, endpoint, new_buffer
					)
				)
			);
		}
	}

	if( bytes_sent ) {
		OnSent();
	}
}

void UdpSocket::ReceiveHandler( const asio::error_code& error, std::size_t bytes_received, std::shared_ptr<asio::ip::udp::endpoint> endpoint_ptr ) {
	{
		auto lock = AcquireLock();

		if( m_receiving ) {
			return;
		}

		if( ( error == asio::error::operation_aborted ) || ( error == asio::error::connection_aborted ) || ( error == asio::error::connection_reset ) ) {
			return;
		}
		else if( error ) {
			std::cerr << "Async Receive Error: " << error.message() << "\n";
			return;
		}

		if( bytes_received ) {
			auto endpoint = *endpoint_ptr;

			m_receive_buffer[endpoint].insert( m_receive_buffer[endpoint].end(), m_receive_memory.begin(), m_receive_memory.begin() + bytes_received );

			m_pending_data += bytes_received;
		}

		if( m_pending_data < SFNUL_MAX_BUFFER_DATA_SIZE ) {
			std::shared_ptr<asio::ip::udp::endpoint> receive_endpoint_ptr = std::make_shared<asio::ip::udp::endpoint>();

			m_receiving = true;

			m_socket.async_receive_from( asio::buffer( m_receive_memory ), *receive_endpoint_ptr,
				m_strand.wrap(
					std::bind(
						[]( std::weak_ptr<UdpSocket> socket, const asio::error_code& handler_error, std::size_t handler_bytes_received, std::shared_ptr<asio::ip::udp::endpoint> handler_endpoint_ptr ) {
							auto shared_socket = socket.lock();

							if( !shared_socket ) {
								return;
							}

							auto handler_lock = shared_socket->AcquireLock();
							shared_socket->m_receiving = false;
							shared_socket->ReceiveHandler( handler_error, handler_bytes_received, handler_endpoint_ptr );
						},
						std::weak_ptr<UdpSocket>( shared_from_this() ), std::placeholders::_1, std::placeholders::_2, receive_endpoint_ptr
					)
				)
			);
		}
	}

	if( bytes_received ) {
		OnReceived();
	}
}

void UdpSocket::SendTo( const void* data, std::size_t size, const Endpoint& endpoint ) {
	auto lock = AcquireLock();

	if( !data || !size ) {
		return;
	}

	if( !m_socket.is_open() ) {
		m_socket.open( endpoint.GetInternalEndpoint<asio::ip::udp>().protocol() );
	}

	auto asio_endpoint = endpoint.GetInternalEndpoint<asio::ip::udp>();
	auto buffer = std::make_shared<std::vector<char>>( size );

	std::memcpy( &( (*buffer)[0] ), data, size );

	SendHandler( asio::error_code{}, 0, asio_endpoint, buffer );
}

std::size_t UdpSocket::ReceiveFrom( void* data, std::size_t size, const Endpoint& endpoint ) {
	auto lock = AcquireLock();

	if( !data || !size ) {
		return 0;
	}

	if( !m_socket.is_open() ) {
		m_socket.open( endpoint.GetInternalEndpoint<asio::ip::udp>().protocol() );
	}

	auto asio_endpoint = endpoint.GetInternalEndpoint<asio::ip::udp>();
	auto receive_size = std::min( size, m_receive_buffer[asio_endpoint].size() );

	for( std::size_t index = 0; index < receive_size; index++ ) {
		static_cast<char*>( data )[index] = m_receive_buffer[asio_endpoint][index];
	}

	m_receive_buffer[asio_endpoint].erase( m_receive_buffer[asio_endpoint].begin(), m_receive_buffer[asio_endpoint].begin() + static_cast<int>( receive_size ) );

	if( m_receive_buffer[asio_endpoint].empty() ) {
		m_receive_buffer.erase( asio_endpoint );
	}

	auto start = false;

	if( ( m_pending_data >= SFNUL_MAX_BUFFER_DATA_SIZE ) && ( m_pending_data - receive_size < SFNUL_MAX_BUFFER_DATA_SIZE ) ) {
		start = true;
	}

	m_pending_data -= receive_size;

	if( start ) {
		ReceiveHandler( asio::error_code{}, 0, nullptr );
	}

	return receive_size;
}

Endpoint UdpSocket::GetLocalEndpoint() const {
	auto lock = AcquireLock();

	return m_socket.local_endpoint();
}

void UdpSocket::ClearBuffers() {
	auto lock = AcquireLock();

	m_receive_buffer.clear();
}

std::size_t UdpSocket::BytesToReceive( const Endpoint& endpoint ) const {
	auto lock = AcquireLock();

	auto asio_endpoint = endpoint.GetInternalEndpoint<asio::ip::udp>();
	auto iterator = m_receive_buffer.find( asio_endpoint );

	if( iterator == m_receive_buffer.end() ) {
		return 0;
	}

	return iterator->second.size();
}

std::deque<Endpoint> UdpSocket::PendingEndpoints() const {
	auto lock = AcquireLock();

	std::deque<Endpoint> endpoints;

	for( auto buffer : m_receive_buffer ) {
		endpoints.push_back( buffer.first );
	}

	return endpoints;
}

void UdpSocket::SetInternalSocket( void* internal_socket ) {
	m_socket = std::move( *static_cast<asio::ip::udp::socket*>( internal_socket ) );
}

}
