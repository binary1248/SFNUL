#include <functional>
#include <algorithm>
#include <iostream>
#include <SFML/Network.hpp>
#include <asio/buffer.hpp>
#include <SFNUL/Endpoint.hpp>
#include <SFNUL/UdpSocket.hpp>

namespace sfn {
namespace {
struct UdpSocketMaker : public UdpSocket {};

struct PacketAccessor : public sf::Packet {
	const void* Send( std::size_t& size ) { return onSend( size ); }
	void Receive( const void* data, std::size_t size ) { onReceive( data, size ); }
};

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
	sf::Lock lock{ m_mutex };

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
	sf::Lock lock{ m_mutex };

	if( !m_socket.is_open() ) {
		return;
	}

	m_socket.close();
}

void UdpSocket::SendHandler( const asio::error_code& error, std::size_t bytes_sent, asio::ip::udp::endpoint endpoint, std::shared_ptr<std::vector<char>> buffer ) {
	sf::Lock lock{ m_mutex };

	if( error == asio::error::operation_aborted ) {
	}
	else if( error ) {
		std::cerr << "Async Send Error: " << error.message() << "\n";
		return;
	}

	if( bytes_sent == buffer->size() ) {
		return;
	}

	auto new_buffer = std::make_shared<std::vector<char>>( buffer->size() - bytes_sent );

	std::copy_n( buffer->begin() + bytes_sent, new_buffer->size(), new_buffer->begin() );

	m_socket.async_send_to( asio::buffer( *new_buffer ), endpoint,
		m_strand.wrap(
			std::bind(
				[]( std::weak_ptr<UdpSocket> socket, const asio::error_code& handler_error, std::size_t handler_bytes_sent, asio::ip::udp::endpoint handler_endpoint, std::shared_ptr<std::vector<char>> handler_buffer ) {
					if( socket.expired() ) {
						return;
					}

					socket.lock()->SendHandler( handler_error, handler_bytes_sent, handler_endpoint, handler_buffer );
				},
				std::weak_ptr<UdpSocket>( shared_from_this() ), std::placeholders::_1, std::placeholders::_2, endpoint, new_buffer
			)
		)
	);
}

void UdpSocket::ReceiveHandler( const asio::error_code& error, std::size_t bytes_received, std::shared_ptr<asio::ip::udp::endpoint> endpoint_ptr ) {
	sf::Lock lock{ m_mutex };

	if( error == asio::error::operation_aborted ) {
	}
	else if( error ) {
		std::cerr << "Async Receive Error: " << error.message() << "\n";
		return;
	}

	if( bytes_received ) {
		if( m_receive_buffer.size() < m_endpoint_limit_hard ) {
			auto endpoint = *endpoint_ptr;

			m_receive_buffer[endpoint].insert( m_receive_buffer[endpoint].end(), m_receive_memory.begin(), m_receive_memory.begin() + bytes_received );

			if( m_receive_buffer.size() > m_endpoint_limit_soft ) {
				std::cerr << "Async Receive Warning: Endpoint count (" << m_receive_buffer.size() << ") exceeds soft limit of " << m_endpoint_limit_soft << ".\n";
			}

			if( m_receive_buffer[endpoint].size() > m_receive_limit_soft ) {
				std::cerr << "Async Receive Warning: Buffer size (" << m_receive_buffer[endpoint].size() << ") exceeds soft limit of " << m_receive_limit_soft << ".\n";
			}

			if( m_receive_buffer[endpoint].size() > m_receive_limit_hard ) {
				m_receive_buffer[endpoint].resize( m_receive_limit_hard );

				std::cerr << "Async Receive Warning: Buffer size (" << m_receive_buffer[endpoint].size() << ") exceeds hard limit of " << m_receive_limit_hard << ". Dropping data.\n";
			}
		}
		else {
			std::cerr << "Async Receive Warning: Endpoint count (" << m_receive_buffer.size() << ") exceeds hard limit of " << m_endpoint_limit_hard << ". Dropping data.\n";
		}
	}

	std::shared_ptr<asio::ip::udp::endpoint> receive_endpoint_ptr = std::make_shared<asio::ip::udp::endpoint>();

	m_socket.async_receive_from( asio::buffer( m_receive_memory ), *receive_endpoint_ptr,
		m_strand.wrap(
			std::bind(
				[]( std::weak_ptr<UdpSocket> socket, const asio::error_code& handler_error, std::size_t handler_bytes_received, std::shared_ptr<asio::ip::udp::endpoint> handler_endpoint_ptr ) {
					if( socket.expired() ) {
						return;
					}

					socket.lock()->ReceiveHandler( handler_error, handler_bytes_received, handler_endpoint_ptr );
				},
				std::weak_ptr<UdpSocket>( shared_from_this() ), std::placeholders::_1, std::placeholders::_2, receive_endpoint_ptr
			)
		)
	);
}

void UdpSocket::SendTo( const void* data, std::size_t size, const Endpoint& endpoint ) {
	sf::Lock lock{ m_mutex };

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
	sf::Lock lock{ m_mutex };

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

	m_receive_buffer[asio_endpoint].erase( m_receive_buffer[asio_endpoint].begin(), m_receive_buffer[asio_endpoint].begin() + receive_size );

	if( m_receive_buffer[asio_endpoint].empty() ) {
		m_receive_buffer.erase( asio_endpoint );
	}

	return receive_size;
}

void UdpSocket::SendTo( sf::Packet& packet, const Endpoint& endpoint ) {
	sf::Lock lock{ m_mutex };

	std::size_t size = 0;
	const void* data = static_cast<PacketAccessor*>( &packet )->Send( size );

	sf::Uint32 packet_size = htonl( static_cast<sf::Uint32>( size ) );

	std::vector<char> data_block( sizeof( packet_size ) + size );

	std::memcpy( &data_block[0], &packet_size, sizeof( packet_size ) );

	if( size ) {
		std::memcpy( &data_block[0] + sizeof( packet_size ), data, size );
	}

	SendTo( &data_block[0], sizeof( packet_size ) + size, endpoint );
}

std::size_t UdpSocket::ReceiveFrom( sf::Packet& packet, const Endpoint& endpoint ) {
	sf::Lock lock{ m_mutex };

	packet.clear();

	sf::Uint32 packet_size = 0;

	auto asio_endpoint = endpoint.GetInternalEndpoint<asio::ip::udp>();

	if( m_receive_buffer[asio_endpoint].size() < sizeof( packet_size ) ) {
		return 0;
	}

	for( std::size_t index = 0; index < sizeof( packet_size ); index++ ) {
		reinterpret_cast<char*>( &packet_size )[index] = m_receive_buffer[asio_endpoint][index];
	}

	packet_size = ntohl( packet_size );

	if( m_receive_buffer[asio_endpoint].size() < sizeof( packet_size ) + packet_size ) {
		return 0;
	}

	std::vector<char> data_block( packet_size );

	m_receive_buffer[asio_endpoint].erase( m_receive_buffer[asio_endpoint].begin(), m_receive_buffer[asio_endpoint].begin() + sizeof( packet_size ) );

	std::copy_n( m_receive_buffer[asio_endpoint].begin(), packet_size, data_block.begin() );

	static_cast<PacketAccessor*>( &packet )->Receive( &data_block[0], packet_size );

	return sizeof( packet_size ) + packet_size;
}

Endpoint UdpSocket::GetLocalEndpoint() const {
	sf::Lock lock{ m_mutex };

	return m_socket.local_endpoint();
}

void UdpSocket::ClearBuffers() {
	sf::Lock lock{ m_mutex };

	m_receive_buffer.clear();
}

std::size_t UdpSocket::BytesToReceive( const Endpoint& endpoint ) const {
	sf::Lock lock{ m_mutex };

	auto asio_endpoint = endpoint.GetInternalEndpoint<asio::ip::udp>();
	auto iterator = m_receive_buffer.find( asio_endpoint );

	if( iterator == m_receive_buffer.end() ) {
		return 0;
	}

	return iterator->second.size();
}

std::deque<Endpoint> UdpSocket::PendingEndpoints() const {
	sf::Lock lock{ m_mutex };

	std::deque<Endpoint> endpoints;

	for( auto buffer : m_receive_buffer ) {
		endpoints.push_back( buffer.first );
	}

	return endpoints;
}

std::size_t UdpSocket::ReceiveSoftLimit( std::size_t limit ) {
	sf::Lock lock{ m_mutex };

	return m_receive_limit_soft = ( limit ? limit : m_receive_limit_soft );
}

std::size_t UdpSocket::ReceiveHardLimit( std::size_t limit ) {
	sf::Lock lock{ m_mutex };

	return m_receive_limit_hard = ( limit ? limit : m_receive_limit_hard );
}

std::size_t UdpSocket::EndpointSoftLimit( std::size_t limit ) {
	sf::Lock lock{ m_mutex };

	return m_endpoint_limit_soft = ( limit ? limit : m_endpoint_limit_soft );
}

std::size_t UdpSocket::EndpointHardLimit( std::size_t limit ) {
	sf::Lock lock{ m_mutex };

	return m_endpoint_limit_hard = ( limit ? limit : m_endpoint_limit_hard );
}

}
