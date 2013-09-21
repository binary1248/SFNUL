#include <functional>
#include <algorithm>
#include <iostream>
#include <SFML/Network.hpp>
#include <SFNUL/Config.hpp>
#include <asio/buffer.hpp>
#include <SFNUL/Endpoint.hpp>
#include <SFNUL/TcpSocket.hpp>

namespace sfn {
namespace {
struct TcpSocketMaker : public TcpSocket {};

struct PacketAccessor : public sf::Packet {
	const void* Send( std::size_t& size ) { return onSend( size ); }
	void Receive( const void* data, std::size_t size ) { onReceive( data, size ); }
};

}

TcpSocket::TcpSocket() :
	m_socket{ GetIOService() },
	m_send_memory{ {} },
	m_receive_memory{ {} }
{
}

TcpSocket::~TcpSocket() {
	Close();
}

TcpSocket::Ptr TcpSocket::Create() {
	return std::make_shared<TcpSocketMaker>();
}

void TcpSocket::Connect( const Endpoint& endpoint ) {
	sf::Lock lock{ m_mutex };

	if( m_connected ) {
		std::cerr << "Connect() Error: Disconnect the current connection before reconnecting.\n";
		return;
	}

	m_send_buffer.clear();
	m_receive_buffer.clear();

	m_socket.async_connect( endpoint.GetInternalEndpoint<asio::ip::tcp>(), [&]( const asio::error_code& error ) {
		if( !error ) {
			OnConnected();

			m_connected = true;
			m_request_shutdown = false;
			m_fin_sent = false;
			m_fin_received = false;

			ReceiveHandler( asio::error_code{}, 0 );
			SendHandler( asio::error_code{}, 0 );
		}
		else if( ( error == asio::error::operation_aborted ) || ( error == asio::error::connection_aborted ) || ( error == asio::error::connection_reset ) ) {
			m_fin_sent = true;
			m_fin_received = true;
			m_connected = false;
		}
		else {
			m_connected = false;



			std::cerr << "Connect() Error: " << error.message() << "\n";
		}
	} );
}

void TcpSocket::Shutdown() {
	sf::Lock lock{ m_mutex };

	if( !m_socket.is_open() ) {
		m_connected = false;

		return;
	}

	if( !m_connected ) {
		std::cerr << "Shutdown() Error: Cannot shutdown unconnected socket.\n";
		return;
	}

	if( m_fin_sent ) {
		return;
	}

	m_request_shutdown = true;

	if( m_send_buffer.empty() ) {
		asio::error_code error;

		m_socket.shutdown( asio::ip::tcp::socket::shutdown_send, error );

		if( ( error == asio::error::connection_aborted ) || ( error == asio::error::connection_reset ) ) {
			m_fin_received = true;
			m_fin_sent = true;

			return;
		}
		else if( error == asio::error::not_connected ) {
		}
		else if( error ) {
			std::cerr << "Shutdown() Error: " << error.message() << "\n";
			return;
		}

		m_fin_sent = true;
	}
}

void TcpSocket::Close() {
	{
		sf::Lock lock{ m_mutex };

		if( !m_socket.is_open() ) {
			m_connected = false;
			return;
		}

		if( !m_fin_sent ) {
			asio::error_code error;

			m_socket.shutdown( asio::ip::tcp::socket::shutdown_send, error );

			if( ( error == asio::error::connection_aborted ) || ( error == asio::error::connection_reset ) ) {
				m_fin_received = true;
			}
			else if( error == asio::error::not_connected ) {
			}
			else if( error ) {
				std::cerr << "Shutdown() Error: " << error.message() << "\n";
				return;
			}

			m_fin_sent = true;

			if( !m_send_buffer.empty() ) {
				std::cerr << "Close(): Warning, did not send all data before shutdown, possible data loss might occur.\n";
			}
		}

		if( !m_fin_received ) {
			std::cerr << "Close(): Warning, the remote host did not request connection shutdown, possible data loss might occur.\n";
		}

		m_connected = false;

		asio::error_code error;

		m_socket.shutdown( asio::ip::tcp::socket::shutdown_both, error );

		if( error == asio::error::not_connected ) {
		}
		else if( error ) {
			std::cerr << "Close() Error: " << error.message() << "\n";
		}

		m_socket.close();
	}

	OnDisconnected();
}

void TcpSocket::SendHandler( const asio::error_code& error, std::size_t bytes_sent ) {
	{
		sf::Lock lock{ m_mutex };

		if( ( error == asio::error::operation_aborted ) || ( error == asio::error::connection_aborted ) || ( error == asio::error::connection_reset ) ) {
			return;
		}
		else if( error ) {
			std::cerr << "Async Send Error: " << error.message() << "\n";
			return;
		}

		m_send_buffer.erase( m_send_buffer.begin(), m_send_buffer.begin() + bytes_sent );

		if( m_send_buffer.empty() ) {
			if( m_request_shutdown && !m_fin_sent && m_socket.is_open() ) {
				asio::error_code shutdown_error;

				m_socket.shutdown( asio::ip::tcp::socket::shutdown_send, shutdown_error );

				if( ( shutdown_error == asio::error::connection_aborted ) || ( shutdown_error == asio::error::connection_reset ) ) {
					m_fin_received = true;
					m_fin_sent = true;

					Close();
				}
				else if( error == asio::error::not_connected ) {
				}
				else if( shutdown_error ) {
					std::cerr << "Shutdown() Error: " << shutdown_error.message() << "\n";
				}
				else {
					m_fin_sent = true;
				}
			}
		}
		else {
			auto send_size = std::min( m_send_memory.size(), m_send_buffer.size() );
			std::copy_n( m_send_buffer.begin(), send_size, m_send_memory.begin() );

			if( m_connected && !m_fin_sent ) {
				m_socket.async_send( asio::buffer( m_send_memory, send_size ),
					m_strand.wrap(
						std::bind(
							[]( std::weak_ptr<TcpSocket> socket, const asio::error_code& handler_error, std::size_t handler_bytes_received ) {
								if( socket.expired() ) {
									return;
								}

								socket.lock()->SendHandler( handler_error, handler_bytes_received );
							},
							std::weak_ptr<TcpSocket>( shared_from_this() ), std::placeholders::_1, std::placeholders::_2
						)
					)
				);
			}
		}
	}

	if( bytes_sent ) {
		OnSent();
	}
}

void TcpSocket::ReceiveHandler( const asio::error_code& error, std::size_t bytes_received ) {
	{
		sf::Lock lock{ m_mutex };

		if( error == asio::error::operation_aborted ) {
			return;
		}
		else if( ( error == asio::error::connection_aborted ) || ( error == asio::error::connection_reset ) ) {
			m_fin_received = true;
			m_fin_sent = true;

			m_send_buffer.clear();

			Close();
		}
		else if( error == asio::error::eof ) {
			m_fin_received = true;
		}
		else if( error ) {
			std::cerr << "Async Receive Error: " << error.message() << "\n";
			return;
		}

		m_receive_buffer.insert( m_receive_buffer.end(), m_receive_memory.begin(), m_receive_memory.begin() + bytes_received );

		if( m_receive_buffer.size() > m_receive_limit_soft ) {
			std::cerr << "Async Receive Warning: Buffer size (" << m_receive_buffer.size() << ") exceeds soft limit of " << m_receive_limit_soft << ".\n";
		}

		if( m_receive_buffer.size() > m_receive_limit_hard ) {
			m_receive_buffer.resize( m_receive_limit_hard );

			std::cerr << "Async Receive Warning: Buffer size (" << m_receive_buffer.size() << ") exceeds hard limit of " << m_receive_limit_hard << ". Dropping data.\n";
		}

		if( m_connected && !m_fin_received ) {
			m_socket.async_receive( asio::buffer( m_receive_memory ),
				m_strand.wrap(
					std::bind(
						[]( std::weak_ptr<TcpSocket> socket, const asio::error_code& handler_error, std::size_t handler_bytes_received ) {
							if( socket.expired() ) {
								return;
							}

							socket.lock()->ReceiveHandler( handler_error, handler_bytes_received );
						},
						std::weak_ptr<TcpSocket>( shared_from_this() ), std::placeholders::_1, std::placeholders::_2
					)
				)
			);
		}
	}

	if( bytes_received ) {
		OnReceived();
	}
}

bool TcpSocket::LocalHasShutdown() const {
	sf::Lock lock{ m_mutex };

	return m_fin_sent;
}

bool TcpSocket::RemoteHasShutdown() const {
	sf::Lock lock{ m_mutex };

	return m_fin_received;
}

void TcpSocket::Send( const void* data, std::size_t size ) {
	sf::Lock lock{ m_mutex };

	if( !data || !size ) {
		return;
	}

	if( m_request_shutdown ) {
		std::cerr << "Send() Error: Cannot send data after shutdown.\n";
		return;
	}

	bool start = m_send_buffer.empty();

	m_send_buffer.insert( m_send_buffer.end(), static_cast<const char*>( data ), static_cast<const char*>( data ) + size );

	if( m_send_buffer.size() > m_send_limit_soft ) {
		std::cerr << "Send() Warning: Buffer size (" << m_send_buffer.size() << ") exceeds soft limit of " << m_send_limit_soft << ".\n";
	}

	if( m_send_buffer.size() > m_send_limit_hard ) {
		m_send_buffer.resize( m_send_limit_hard );

		std::cerr << "Send() Warning: Buffer size (" << m_send_buffer.size() << ") exceeds hard limit of " << m_send_limit_hard << ". Dropping data.\n";
	}

	if( start ) {
		SendHandler( asio::error_code{}, 0 );
	}
}

std::size_t TcpSocket::Receive( void* data, std::size_t size ) {
	sf::Lock lock{ m_mutex };

	if( !data || !size ) {
		return 0;
	}

	auto receive_size = std::min( size, m_receive_buffer.size() );

	for( std::size_t index = 0; index < receive_size; index++ ) {
		static_cast<char*>( data )[index] = m_receive_buffer[index];
	}

	m_receive_buffer.erase( m_receive_buffer.begin(), m_receive_buffer.begin() + receive_size );

	return receive_size;
}

void TcpSocket::Send( sf::Packet& packet ) {
	sf::Lock lock{ m_mutex };

	std::size_t size = 0;
	const void* data = static_cast<PacketAccessor*>( &packet )->Send( size );

	sf::Uint32 packet_size = htonl( static_cast<sf::Uint32>( size ) );

	std::vector<char> data_block( sizeof( packet_size ) + size );

	std::memcpy( &data_block[0], &packet_size, sizeof( packet_size ) );

	if( size ) {
		std::memcpy( &data_block[0] + sizeof( packet_size ), data, size );
	}

	Send( &data_block[0], sizeof( packet_size ) + size );
}

std::size_t TcpSocket::Receive( sf::Packet& packet ) {
	sf::Lock lock{ m_mutex };

	packet.clear();

	sf::Uint32 packet_size = 0;

	if( m_receive_buffer.size() < sizeof( packet_size ) ) {
		return 0;
	}

	for( std::size_t index = 0; index < sizeof( packet_size ); index++ ) {
		reinterpret_cast<char*>( &packet_size )[index] = m_receive_buffer[index];
	}

	packet_size = ntohl( packet_size );

	if( m_receive_buffer.size() < sizeof( packet_size ) + packet_size ) {
		return 0;
	}

	std::vector<char> data_block( packet_size );

	m_receive_buffer.erase( m_receive_buffer.begin(), m_receive_buffer.begin() + sizeof( packet_size ) );

	std::copy_n( m_receive_buffer.begin(), packet_size, data_block.begin() );

	static_cast<PacketAccessor*>( &packet )->Receive( &data_block[0], packet_size );

	return sizeof( packet_size ) + packet_size;
}

bool TcpSocket::IsConnected() const {
	sf::Lock lock{ m_mutex };

	return m_connected;
}

Endpoint TcpSocket::GetLocalEndpoint() const {
	sf::Lock lock{ m_mutex };

	return m_socket.local_endpoint();
}

Endpoint TcpSocket::GetRemoteEndpoint() const {
	sf::Lock lock{ m_mutex };

	return m_socket.remote_endpoint();
}

void TcpSocket::ClearBuffers() {
	sf::Lock lock{ m_mutex };

	m_send_buffer.clear();
	m_receive_buffer.clear();
}

std::size_t TcpSocket::BytesToSend() const {
	sf::Lock lock{ m_mutex };

	return m_send_buffer.size();
}

std::size_t TcpSocket::BytesToReceive() const {
	sf::Lock lock{ m_mutex };

	return m_receive_buffer.size();
}

std::size_t TcpSocket::SendSoftLimit( std::size_t limit ) {
	sf::Lock lock{ m_mutex };

	return m_send_limit_soft = ( limit ? limit : m_send_limit_soft );
}

std::size_t TcpSocket::SendHardLimit( std::size_t limit ) {
	sf::Lock lock{ m_mutex };

	return m_send_limit_hard = ( limit ? limit : m_send_limit_hard );
}

std::size_t TcpSocket::ReceiveSoftLimit( std::size_t limit ) {
	sf::Lock lock{ m_mutex };

	return m_receive_limit_soft = ( limit ? limit : m_receive_limit_soft );
}

std::size_t TcpSocket::ReceiveHardLimit( std::size_t limit ) {
	sf::Lock lock{ m_mutex };

	return m_receive_limit_hard = ( limit ? limit : m_receive_limit_hard );
}

int TcpSocket::GetLinger() const {
	sf::Lock lock{ m_mutex };

	asio::socket_base::linger option;

	asio::error_code error;

	m_socket.get_option( option, error );

	if( error ) {
		std::cerr << "GetLinger() Error: " << error.message() << "\n";
	}

	return option.enabled() ? option.timeout() : 0;
}

void TcpSocket::SetLinger( int timeout ) {
	sf::Lock lock{ m_mutex };

	asio::socket_base::linger option( timeout > 0, timeout );

	asio::error_code error;

	m_socket.set_option( option, error );

	if( error ) {
		std::cerr << "SetLinger() Error: " << error.message() << "\n";
	}
}

bool TcpSocket::GetKeepAlive() const {
	sf::Lock lock{ m_mutex };

	asio::socket_base::keep_alive option;

	asio::error_code error;

	m_socket.get_option( option, error );

	if( error ) {
		std::cerr << "GetKeepAlive() Error: " << error.message() << "\n";
	}

	return option.value();
}

void TcpSocket::SetKeepAlive( bool keep_alive ) {
	sf::Lock lock{ m_mutex };

	asio::socket_base::keep_alive option( keep_alive );

	asio::error_code error;

	m_socket.set_option( option, error );

	if( error ) {
		std::cerr << "SetKeepAlive() Error: " << error.message() << "\n";
	}
}

void TcpSocket::SetInternalSocket( void* internal_socket ) {
	m_socket = std::move( *static_cast<asio::ip::tcp::socket*>( internal_socket ) );

	OnConnected();

	m_connected = true;
	m_request_shutdown = false;
	m_fin_sent = false;
	m_fin_received = false;

	ReceiveHandler( asio::error_code{}, 0 );
	SendHandler( asio::error_code{}, 0 );
}

}
