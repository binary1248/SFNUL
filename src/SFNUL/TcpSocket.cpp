/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <functional>
#include <algorithm>
#include <iostream>
#include <SFNUL/Config.hpp>
#include <asio/buffer.hpp>
#include <SFNUL/Endpoint.hpp>
#include <SFNUL/TcpSocket.hpp>
#include <SFNUL/Message.hpp>

namespace sfn {
namespace {
struct TcpSocketMaker : public TcpSocket {};
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
	auto lock = AcquireLock();

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
	auto lock = AcquireLock();

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
		auto lock = AcquireLock();

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

/// @cond
void TcpSocket::Reset() {
	m_fin_received = true;
}
/// @endcond

void TcpSocket::SendHandler( const asio::error_code& error, std::size_t bytes_sent ) {
	{
		auto lock = AcquireLock();

		if( m_sending ) {
			return;
		}

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
				m_sending = true;

				m_socket.async_send( asio::buffer( m_send_memory, send_size ),
					m_strand.wrap(
						std::bind(
							[]( std::weak_ptr<TcpSocket> socket, const asio::error_code& handler_error, std::size_t handler_bytes_received ) {
								auto shared_socket = socket.lock();

								if( !shared_socket ) {
									return;
								}

								auto handler_lock = shared_socket->AcquireLock();
								shared_socket->m_sending = false;

								if( !shared_socket->m_send_buffer.empty() ) {
									shared_socket->SendHandler( handler_error, handler_bytes_received );
								}
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
		auto lock = AcquireLock();

		if( m_receiving ) {
			return;
		}

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

		if( m_connected && !m_fin_received && ( m_receive_buffer.size() < SFNUL_MAX_BUFFER_DATA_SIZE ) ) {
			m_receiving = true;

			m_socket.async_receive( asio::buffer( m_receive_memory ),
				m_strand.wrap(
					std::bind(
						[]( std::weak_ptr<TcpSocket> socket, const asio::error_code& handler_error, std::size_t handler_bytes_received ) {
							auto shared_socket = socket.lock();

							if( !shared_socket ) {
								return;
							}

							auto handler_lock = shared_socket->AcquireLock();
							shared_socket->m_receiving = false;
							shared_socket->ReceiveHandler( handler_error, handler_bytes_received );
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
	auto lock = AcquireLock();

	return m_fin_sent;
}

bool TcpSocket::RemoteHasShutdown() const {
	auto lock = AcquireLock();

	return m_fin_received;
}

bool TcpSocket::Send( const void* data, std::size_t size ) {
	auto lock = AcquireLock();

	if( !data || !size ) {
		return false;
	}

	if( m_request_shutdown ) {
		std::cerr << "Send() Error: Cannot send data after shutdown.\n";
		return false;
	}

	if( m_send_buffer.size() + size >= SFNUL_MAX_BUFFER_DATA_SIZE ) {
		return false;
	}

	bool start = m_send_buffer.empty();

	m_send_buffer.insert( m_send_buffer.end(), static_cast<const char*>( data ), static_cast<const char*>( data ) + size );

	if( start ) {
		SendHandler( asio::error_code{}, 0 );
	}

	return true;
}

std::size_t TcpSocket::Receive( void* data, std::size_t size ) {
	auto lock = AcquireLock();

	if( !data || !size ) {
		return 0;
	}

	auto receive_size = std::min( size, m_receive_buffer.size() );

	for( std::size_t index = 0; index < receive_size; index++ ) {
		static_cast<char*>( data )[index] = m_receive_buffer[index];
	}

	auto start = false;

	if( ( m_receive_buffer.size() >= SFNUL_MAX_BUFFER_DATA_SIZE ) && ( m_receive_buffer.size() - receive_size ) < SFNUL_MAX_BUFFER_DATA_SIZE ) {
		start = true;
	}

	m_receive_buffer.erase( m_receive_buffer.begin(), m_receive_buffer.begin() + receive_size );

	if( start ) {
		ReceiveHandler( asio::error_code{}, 0 );
	}

	return receive_size;
}

bool TcpSocket::Send( const Message& message ) {
	auto lock = AcquireLock();

	auto message_size = message.GetSize();

	if( m_send_buffer.size() + sizeof( message_size ) + message_size >= SFNUL_MAX_BUFFER_DATA_SIZE ) {
		return false;
	}

	std::vector<char> data_block( sizeof( message_size ) + message_size );

	std::memcpy( &data_block[0], &message_size, sizeof( message_size ) );

	if( message_size ) {
		const auto& message_data = message.GetBuffer();
		std::copy( std::begin( message_data ), std::end( message_data ), std::begin( data_block ) + sizeof( message_size ) );
	}

	return Send( &data_block[0], sizeof( message_size ) + static_cast<std::size_t>( message_size ) );
}

std::size_t TcpSocket::Receive( Message& message ) {
	auto lock = AcquireLock();

	message.Clear();

	Message::size_type message_size = 0;

	if( m_receive_buffer.size() < sizeof( message_size ) ) {
		return 0;
	}

	for( std::size_t index = 0; index < sizeof( message_size ); index++ ) {
		reinterpret_cast<char*>( &message_size )[index] = m_receive_buffer[index];
	}

	if( m_receive_buffer.size() < sizeof( message_size ) + message_size ) {
		return 0;
	}

	auto start = false;

	if( ( m_receive_buffer.size() >= SFNUL_MAX_BUFFER_DATA_SIZE ) && ( m_receive_buffer.size() - ( sizeof( message_size ) + message_size ) ) < SFNUL_MAX_BUFFER_DATA_SIZE ) {
		start = true;
	}

	std::vector<char> data_block( message_size );

	m_receive_buffer.erase( m_receive_buffer.begin(), m_receive_buffer.begin() + sizeof( message_size ) );

	std::copy_n( m_receive_buffer.begin(), message_size, data_block.begin() );

	message.Append( data_block );

	m_receive_buffer.erase( m_receive_buffer.begin(), m_receive_buffer.begin() + message_size );

	if( start ) {
		ReceiveHandler( asio::error_code{}, 0 );
	}

	return sizeof( message_size ) + message_size;
}

bool TcpSocket::IsConnected() const {
	auto lock = AcquireLock();

	return m_connected;
}

Endpoint TcpSocket::GetLocalEndpoint() const {
	auto lock = AcquireLock();

	return m_socket.local_endpoint();
}

Endpoint TcpSocket::GetRemoteEndpoint() const {
	auto lock = AcquireLock();

	return m_socket.remote_endpoint();
}

void TcpSocket::ClearBuffers() {
	auto lock = AcquireLock();

	m_send_buffer.clear();
	m_receive_buffer.clear();

	ReceiveHandler( asio::error_code{}, 0 );
}

std::size_t TcpSocket::BytesToSend() const {
	auto lock = AcquireLock();

	return m_send_buffer.size();
}

std::size_t TcpSocket::BytesToReceive() const {
	auto lock = AcquireLock();

	return m_receive_buffer.size();
}

int TcpSocket::GetLinger() const {
	auto lock = AcquireLock();

	asio::socket_base::linger option;

	asio::error_code error;

	m_socket.get_option( option, error );

	if( error ) {
		std::cerr << "GetLinger() Error: " << error.message() << "\n";
	}

	return option.enabled() ? option.timeout() : 0;
}

void TcpSocket::SetLinger( int timeout ) {
	auto lock = AcquireLock();

	asio::socket_base::linger option( timeout > 0, timeout );

	asio::error_code error;

	m_socket.set_option( option, error );

	if( error ) {
		std::cerr << "SetLinger() Error: " << error.message() << "\n";
	}
}

bool TcpSocket::GetKeepAlive() const {
	auto lock = AcquireLock();

	asio::socket_base::keep_alive option;

	asio::error_code error;

	m_socket.get_option( option, error );

	if( error ) {
		std::cerr << "GetKeepAlive() Error: " << error.message() << "\n";
	}

	return option.value();
}

void TcpSocket::SetKeepAlive( bool keep_alive ) {
	auto lock = AcquireLock();

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
