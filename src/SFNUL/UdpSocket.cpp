/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <SFNUL/UdpSocket.hpp>
#include <SFNUL/ConfigInternal.hpp>
#include <SFNUL/Utility.hpp>
#include <SFNUL/Endpoint.hpp>
#include <SFNUL/IpAddressImpl.hpp>
#include <SFNUL/MakeUnique.hpp>
#include <asio/buffer.hpp>
#include <asio/ip/udp.hpp>
#include <asio/strand.hpp>
#include <functional>
#include <algorithm>

namespace {

struct UdpSocketMaker : public sfn::UdpSocket {};

}

namespace sfn {

class UdpSocket::UdpSocketImpl {
public:
	UdpSocketImpl( UdpSocket* owner ) :
		udp_socket{ owner },
		asio_socket{ *static_cast<asio::io_service*>( owner->GetIOService() ) }
	{

	}

	void SendHandler( const asio::error_code& error, std::size_t bytes_sent, asio::ip::udp::endpoint endpoint, std::shared_ptr<std::vector<char>> buffer ) {
		{
			auto lock = udp_socket->AcquireLock();

			if( ( error == asio::error::operation_aborted ) || ( error == asio::error::connection_aborted ) || ( error == asio::error::connection_reset ) ) {
				return;
			}
			else if( error ) {
				ErrorMessage() << "Async Send Error: " << error.message() << "\n";
				return;
			}

			if( bytes_sent == buffer->size() ) {
				return;
			}
			else {
				auto new_buffer = std::make_shared<std::vector<char>>( buffer->size() - bytes_sent );

				std::copy_n( buffer->begin() + static_cast<int>( bytes_sent ), new_buffer->size(), new_buffer->begin() );

				asio_socket.async_send_to( asio::buffer( *new_buffer ), endpoint,
					static_cast<asio::strand*>( udp_socket->GetStrand() )->wrap(
						std::bind(
							[]( std::weak_ptr<UdpSocket> socket, const asio::error_code& handler_error, std::size_t handler_bytes_sent, asio::ip::udp::endpoint handler_endpoint, std::shared_ptr<std::vector<char>> handler_buffer ) {
								auto shared_socket = socket.lock();

								if( !shared_socket ) {
									return;
								}

								auto handler_lock = shared_socket->AcquireLock();
								shared_socket->m_impl->SendHandler( handler_error, handler_bytes_sent, handler_endpoint, handler_buffer );
							},
							std::weak_ptr<UdpSocket>( udp_socket->shared_from_this() ), std::placeholders::_1, std::placeholders::_2, endpoint, new_buffer
						)
					)
				);
			}
		}

		if( bytes_sent ) {
			udp_socket->OnSent();
		}
	}

	void ReceiveHandler( const asio::error_code& error, std::size_t bytes_received, std::shared_ptr<asio::ip::udp::endpoint> endpoint_ptr ) {
		{
			auto lock = udp_socket->AcquireLock();

			if( receiving ) {
				return;
			}

			if( ( error == asio::error::operation_aborted ) || ( error == asio::error::connection_aborted ) || ( error == asio::error::connection_reset ) ) {
				return;
			}
			else if( error ) {
				ErrorMessage() << "Async Receive Error: " << error.message() << "\n";
				return;
			}

			if( bytes_received ) {
				auto endpoint = *endpoint_ptr;

				receive_buffer[endpoint].insert( receive_buffer[endpoint].end(), receive_memory.begin(), receive_memory.begin() + bytes_received );

				pending_data += bytes_received;
			}

			if( pending_data < GetMaximumBlockSize() ) {
				std::shared_ptr<asio::ip::udp::endpoint> receive_endpoint_ptr = std::make_shared<asio::ip::udp::endpoint>();

				receiving = true;

				asio_socket.async_receive_from( asio::buffer( receive_memory ), *receive_endpoint_ptr,
					static_cast<asio::strand*>( udp_socket->GetStrand() )->wrap(
						std::bind(
							[]( std::weak_ptr<UdpSocket> socket, const asio::error_code& handler_error, std::size_t handler_bytes_received, std::shared_ptr<asio::ip::udp::endpoint> handler_endpoint_ptr ) {
								auto shared_socket = socket.lock();

								if( !shared_socket ) {
									return;
								}

								auto handler_lock = shared_socket->AcquireLock();
								shared_socket->m_impl->receiving = false;
								shared_socket->m_impl->ReceiveHandler( handler_error, handler_bytes_received, handler_endpoint_ptr );
							},
							std::weak_ptr<UdpSocket>( udp_socket->shared_from_this() ), std::placeholders::_1, std::placeholders::_2, receive_endpoint_ptr
						)
					)
				);
			}
		}

		if( bytes_received ) {
			udp_socket->OnReceived();
		}
	}

	UdpSocket* udp_socket;

	asio::ip::udp::socket asio_socket;

	std::map<asio::ip::udp::endpoint, std::vector<char>> receive_buffer;

	std::array<char, 2048> receive_memory;

	std::size_t pending_data = 0;

	bool receiving = false;
};

UdpSocket::UdpSocket() :
	m_impl{ make_unique<UdpSocketImpl>( this ) }
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

	auto asio_endpoint = asio::ip::basic_endpoint<asio::ip::udp>{ endpoint.GetAddress().m_impl->address, endpoint.GetPort() };

	if( !m_impl->asio_socket.is_open() ) {
		m_impl->asio_socket.open( asio_endpoint.protocol() );
	}

	asio::error_code error;

	m_impl->asio_socket.bind( asio_endpoint, error );

	if( error ) {
		ErrorMessage() << "Bind() Error: " << error.message() << "\n";
		return;
	}

	m_impl->ReceiveHandler( asio::error_code{}, 0, nullptr );
}

void UdpSocket::Close() {
	auto lock = AcquireLock();

	if( !m_impl->asio_socket.is_open() ) {
		return;
	}

	m_impl->asio_socket.close();
}

void UdpSocket::SendTo( const void* data, std::size_t size, const Endpoint& endpoint ) {
	auto lock = AcquireLock();

	if( !data || !size ) {
		return;
	}

	auto asio_endpoint = asio::ip::basic_endpoint<asio::ip::udp>{ endpoint.GetAddress().m_impl->address, endpoint.GetPort() };

	if( !m_impl->asio_socket.is_open() ) {
		m_impl->asio_socket.open( asio_endpoint.protocol() );
	}

	auto buffer = std::make_shared<std::vector<char>>( size );

	std::memcpy( &( (*buffer)[0] ), data, size );

	m_impl->SendHandler( asio::error_code{}, 0, asio_endpoint, buffer );
}

std::size_t UdpSocket::ReceiveFrom( void* data, std::size_t size, const Endpoint& endpoint ) {
	auto lock = AcquireLock();

	if( !data || !size ) {
		return 0;
	}

	auto asio_endpoint = asio::ip::basic_endpoint<asio::ip::udp>{ endpoint.GetAddress().m_impl->address, endpoint.GetPort() };

	if( !m_impl->asio_socket.is_open() ) {
		m_impl->asio_socket.open( asio_endpoint.protocol() );
	}

	auto receive_size = std::min( size, m_impl->receive_buffer[asio_endpoint].size() );

	for( std::size_t index = 0; index < receive_size; index++ ) {
		static_cast<char*>( data )[index] = m_impl->receive_buffer[asio_endpoint][index];
	}

	m_impl->receive_buffer[asio_endpoint].erase( m_impl->receive_buffer[asio_endpoint].begin(), m_impl->receive_buffer[asio_endpoint].begin() + static_cast<int>( receive_size ) );

	if( m_impl->receive_buffer[asio_endpoint].empty() ) {
		m_impl->receive_buffer.erase( asio_endpoint );
	}

	auto start = false;

	if( ( m_impl->pending_data >= GetMaximumBlockSize() ) && ( m_impl->pending_data - receive_size < GetMaximumBlockSize() ) ) {
		start = true;
	}

	m_impl->pending_data -= receive_size;

	if( start ) {
		m_impl->ReceiveHandler( asio::error_code{}, 0, nullptr );
	}

	return receive_size;
}

Endpoint UdpSocket::GetLocalEndpoint() const {
	auto lock = AcquireLock();

	IpAddress address;
	address.m_impl->address = m_impl->asio_socket.local_endpoint().address();

	return { address, m_impl->asio_socket.local_endpoint().port() };
}

void UdpSocket::ClearBuffers() {
	auto lock = AcquireLock();

	m_impl->receive_buffer.clear();
}

std::size_t UdpSocket::BytesToReceive( const Endpoint& endpoint ) const {
	auto lock = AcquireLock();

	auto asio_endpoint = asio::ip::basic_endpoint<asio::ip::udp>{ endpoint.GetAddress().m_impl->address, endpoint.GetPort() };
	auto iterator = m_impl->receive_buffer.find( asio_endpoint );

	if( iterator == m_impl->receive_buffer.end() ) {
		return 0;
	}

	return iterator->second.size();
}

std::deque<Endpoint> UdpSocket::PendingEndpoints() const {
	auto lock = AcquireLock();

	std::deque<Endpoint> endpoints;

	for( auto buffer : m_impl->receive_buffer ) {
		IpAddress address;
		address.m_impl->address = buffer.first.address();

		endpoints.emplace_back( address, buffer.first.port() );
	}

	return endpoints;
}

/// @cond
void UdpSocket::SetInternalSocket( void* internal_socket ) {
	m_impl->asio_socket = std::move( *static_cast<asio::ip::udp::socket*>( internal_socket ) );
}
/// @endcond

}
