/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <SFNUL/TcpSocket.hpp>
#include <SFNUL/ConfigInternal.hpp>
#include <SFNUL/Utility.hpp>
#include <SFNUL/Endpoint.hpp>
#include <SFNUL/Message.hpp>
#include <SFNUL/IpAddressImpl.hpp>
#include <SFNUL/MakeUnique.hpp>
#include <asio/buffer.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/strand.hpp>
#include <functional>
#include <algorithm>

namespace {

struct TcpSocketMaker : public sfn::TcpSocket {};

}

namespace sfn {

class TcpSocket::TcpSocketImpl {
public:
	TcpSocketImpl( TcpSocket* owner ) :
		tcp_socket{ owner },
		asio_socket{ *static_cast<asio::io_context*>( owner->GetIOService() ) }
	{
	}

	void SendHandler( const asio::error_code& error, std::size_t bytes_sent ) {
		{
			auto lock = tcp_socket->AcquireLock();

			if( sending ) {
				return;
			}

			if( ( error == asio::error::operation_aborted ) || ( error == asio::error::connection_aborted ) || ( error == asio::error::connection_reset ) ) {
				return;
			}
			else if( error ) {
				ErrorMessage() << "Async Send Error: " << error.message() << "\n";
				return;
			}

			send_buffer.erase( send_buffer.begin(), send_buffer.begin() + static_cast<int>( bytes_sent ) );

			if( send_buffer.empty() ) {
				if( request_shutdown && !fin_sent && asio_socket.is_open() ) {
					asio::error_code shutdown_error;

					asio_socket.shutdown( asio::ip::tcp::socket::shutdown_send, shutdown_error );

					if( ( shutdown_error == asio::error::connection_aborted ) || ( shutdown_error == asio::error::connection_reset ) ) {
						fin_received = true;
						fin_sent = true;

						tcp_socket->Close();
					}
					else if( error == asio::error::not_connected ) {
					}
					else if( shutdown_error ) {
						ErrorMessage() << "Shutdown() Error: " << shutdown_error.message() << "\n";
					}
					else {
						fin_sent = true;
					}
				}
			}
			else {
				auto send_size = std::min( send_memory.size(), send_buffer.size() );
				std::copy_n( send_buffer.begin(), send_size, send_memory.begin() );

				if( connected && !fin_sent ) {
					sending = true;

					asio_socket.async_send( asio::buffer( send_memory, send_size ),
						static_cast<asio::io_context::strand*>( tcp_socket->GetStrand() )->wrap(
							std::bind(
								[]( std::weak_ptr<TcpSocket> socket, const asio::error_code& handler_error, std::size_t handler_bytes_sent ) {
									auto shared_socket = socket.lock();

									if( !shared_socket ) {
										return;
									}

									auto handler_lock = shared_socket->AcquireLock();
									shared_socket->m_impl->sending = false;

									if( !shared_socket->m_impl->send_buffer.empty() ) {
										shared_socket->m_impl->SendHandler( handler_error, handler_bytes_sent );
									}
								},
								std::weak_ptr<TcpSocket>( tcp_socket->shared_from_this() ), std::placeholders::_1, std::placeholders::_2
							)
						)
					);
				}
			}
		}

		if( bytes_sent ) {
			tcp_socket->OnSent();
		}
	}

	void ReceiveHandler( const asio::error_code& error, std::size_t bytes_received ) {
		{
			auto lock = tcp_socket->AcquireLock();

			if( receiving ) {
				return;
			}

			if( error == asio::error::operation_aborted ) {
				return;
			}
			else if( ( error == asio::error::connection_aborted ) || ( error == asio::error::connection_reset ) ) {
				fin_received = true;
				fin_sent = true;

				send_buffer.clear();

				tcp_socket->Close();
			}
			else if( error == asio::error::eof ) {
				fin_received = true;
			}
			else if( error ) {
				ErrorMessage() << "Async Receive Error: " << error.message() << "\n";
				return;
			}

			receive_buffer.insert( receive_buffer.end(), receive_memory.begin(), receive_memory.begin() + bytes_received );

			if( connected && !fin_received && ( receive_buffer.size() < GetMaximumBlockSize() ) ) {
				receiving = true;

				asio_socket.async_receive( asio::buffer( receive_memory ),
					static_cast<asio::io_context::strand*>( tcp_socket->GetStrand() )->wrap(
						std::bind(
							[]( std::weak_ptr<TcpSocket> socket, const asio::error_code& handler_error, std::size_t handler_bytes_received ) {
								auto shared_socket = socket.lock();

								if( !shared_socket ) {
									return;
								}

								auto handler_lock = shared_socket->AcquireLock();
								shared_socket->m_impl->receiving = false;
								shared_socket->m_impl->ReceiveHandler( handler_error, handler_bytes_received );
							},
							std::weak_ptr<TcpSocket>( tcp_socket->shared_from_this() ), std::placeholders::_1, std::placeholders::_2
						)
					)
				);
			}
		}

		if( bytes_received ) {
			tcp_socket->OnReceived();
		}
	}

	TcpSocket* tcp_socket;

	asio::ip::tcp::socket asio_socket;

	std::vector<char> send_buffer;
	std::vector<char> receive_buffer;

	std::array<char, 2048> send_memory;
	std::array<char, 2048> receive_memory;

	bool connected = false;
	bool request_shutdown = false;
	bool fin_sent = false;
	bool fin_received = false;

	bool receiving = false;
	bool sending = false;
};

TcpSocket::TcpSocket() :
	m_impl{ make_unique<TcpSocketImpl>( this ) }
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

	if( m_impl->connected ) {
		ErrorMessage() << "Connect() Error: Disconnect the current connection before reconnecting.\n";
		return;
	}

	m_impl->send_buffer.clear();
	m_impl->receive_buffer.clear();

	auto asio_endpoint = asio::ip::basic_endpoint<asio::ip::tcp>{ endpoint.GetAddress().m_impl->address, endpoint.GetPort() };

	m_impl->asio_socket.async_connect(
		asio_endpoint,
		static_cast<asio::io_context::strand*>( GetStrand() )->wrap(
			std::bind(
				[&]( std::weak_ptr<TcpSocket> socket, const asio::error_code& error ) {
					auto shared_socket = socket.lock();

					if( !shared_socket ) {
						return;
					}

					auto handler_lock = shared_socket->AcquireLock();

					if( !error ) {
						shared_socket->OnConnected();

						shared_socket->m_impl->connected = true;
						shared_socket->m_impl->request_shutdown = false;
						shared_socket->m_impl->fin_sent = false;
						shared_socket->m_impl->fin_received = false;

						shared_socket->m_impl->ReceiveHandler( asio::error_code{}, 0 );
						shared_socket->m_impl->SendHandler( asio::error_code{}, 0 );
					}
					else if( ( error == asio::error::operation_aborted ) || ( error == asio::error::connection_aborted ) || ( error == asio::error::connection_reset ) ) {
						shared_socket->m_impl->fin_sent = true;
						shared_socket->m_impl->fin_received = true;
						shared_socket->m_impl->connected = false;
					}
					else {
						shared_socket->m_impl->connected = false;

						ErrorMessage() << "Connect() Error: " << error.message() << "\n";
					}
				},
				std::weak_ptr<TcpSocket>( shared_from_this() ), std::placeholders::_1
			)
		)
	);
}

void TcpSocket::Shutdown() {
	auto lock = AcquireLock();

	if( !m_impl->asio_socket.is_open() ) {
		m_impl->connected = false;

		return;
	}

	if( !m_impl->connected ) {
		ErrorMessage() << "Shutdown() Error: Cannot shutdown unconnected socket.\n";
		return;
	}

	if( m_impl->fin_sent ) {
		return;
	}

	m_impl->request_shutdown = true;

	if( m_impl->send_buffer.empty() ) {
		asio::error_code error;

		m_impl->asio_socket.shutdown( asio::ip::tcp::socket::shutdown_send, error );

		if( ( error == asio::error::connection_aborted ) || ( error == asio::error::connection_reset ) ) {
			m_impl->fin_received = true;
			m_impl->fin_sent = true;

			return;
		}
		else if( error == asio::error::not_connected ) {
		}
		else if( error ) {
			ErrorMessage() << "Shutdown() Error: " << error.message() << "\n";
			return;
		}

		m_impl->fin_sent = true;
	}
}

void TcpSocket::Close() {
	{
		auto lock = AcquireLock();

		if( !m_impl->asio_socket.is_open() ) {
			m_impl->connected = false;
			return;
		}

		if( !m_impl->fin_sent ) {
			asio::error_code error;

			m_impl->asio_socket.shutdown( asio::ip::tcp::socket::shutdown_send, error );

			if( ( error == asio::error::connection_aborted ) || ( error == asio::error::connection_reset ) ) {
				m_impl->fin_received = true;
			}
			else if( error == asio::error::not_connected ) {
			}
			else if( error ) {
				ErrorMessage() << "Shutdown() Error: " << error.message() << "\n";
				return;
			}

			m_impl->fin_sent = true;

			if( !m_impl->send_buffer.empty() ) {
				WarningMessage() << "Close(): Warning, did not send all data before shutdown, possible data loss might occur.\n";
			}
		}

		if( !m_impl->fin_received ) {
			WarningMessage() << "Close(): Warning, the remote host did not request connection shutdown, possible data loss might occur.\n";
		}

		m_impl->connected = false;

		asio::error_code error;

		m_impl->asio_socket.shutdown( asio::ip::tcp::socket::shutdown_both, error );

		if( error == asio::error::not_connected ) {
		}
		else if( error ) {
			ErrorMessage() << "Close() Error: " << error.message() << "\n";
		}

		m_impl->asio_socket.close();
	}

	OnDisconnected();
}

/// @cond
void TcpSocket::Reset() {
	m_impl->fin_received = true;
}
/// @endcond

bool TcpSocket::LocalHasShutdown() const {
	auto lock = AcquireLock();

	return m_impl->fin_sent;
}

bool TcpSocket::RemoteHasShutdown() const {
	auto lock = AcquireLock();

	return m_impl->fin_received;
}

bool TcpSocket::Send( const void* data, std::size_t size ) {
	auto lock = AcquireLock();

	if( !data || !size ) {
		return false;
	}

	if( m_impl->request_shutdown ) {
		ErrorMessage() << "Send() Error: Cannot send data after shutdown.\n";
		return false;
	}

	if( m_impl->send_buffer.size() + size >= GetMaximumBlockSize() ) {
		return false;
	}

	bool start = m_impl->send_buffer.empty();

	m_impl->send_buffer.insert( m_impl->send_buffer.end(), static_cast<const char*>( data ), static_cast<const char*>( data ) + size );

	if( start ) {
		m_impl->SendHandler( asio::error_code{}, 0 );
	}

	return true;
}

std::size_t TcpSocket::Receive( void* data, std::size_t size ) {
	auto lock = AcquireLock();

	if( !data || !size ) {
		return 0;
	}

	auto receive_size = std::min( size, m_impl->receive_buffer.size() );

	for( std::size_t index = 0; index < receive_size; index++ ) {
		static_cast<char*>( data )[index] = m_impl->receive_buffer[index];
	}

	auto start = false;

	if( ( m_impl->receive_buffer.size() >= GetMaximumBlockSize() ) && ( m_impl->receive_buffer.size() - receive_size ) < GetMaximumBlockSize() ) {
		start = true;
	}

	m_impl->receive_buffer.erase( m_impl->receive_buffer.begin(), m_impl->receive_buffer.begin() + static_cast<int>( receive_size ) );

	if( start ) {
		m_impl->ReceiveHandler( asio::error_code{}, 0 );
	}

	return receive_size;
}

bool TcpSocket::Send( const Message& message ) {
	auto lock = AcquireLock();

	auto message_size = message.GetSize();

	if( m_impl->send_buffer.size() + sizeof( message_size ) + message_size >= GetMaximumBlockSize() ) {
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

	if( m_impl->receive_buffer.size() < sizeof( message_size ) ) {
		return 0;
	}

	for( std::size_t index = 0; index < sizeof( message_size ); index++ ) {
		reinterpret_cast<char*>( &message_size )[index] = m_impl->receive_buffer[index];
	}

	if( m_impl->receive_buffer.size() < sizeof( message_size ) + message_size ) {
		return 0;
	}

	auto start = false;

	if( ( m_impl->receive_buffer.size() >= GetMaximumBlockSize() ) && ( m_impl->receive_buffer.size() - ( sizeof( message_size ) + message_size ) ) < GetMaximumBlockSize() ) {
		start = true;
	}

	std::vector<char> data_block( message_size );

	m_impl->receive_buffer.erase( m_impl->receive_buffer.begin(), m_impl->receive_buffer.begin() + sizeof( message_size ) );

	std::copy_n( m_impl->receive_buffer.begin(), message_size, data_block.begin() );

	message.Append( data_block );

	m_impl->receive_buffer.erase( m_impl->receive_buffer.begin(), m_impl->receive_buffer.begin() + message_size );

	if( start ) {
		m_impl->ReceiveHandler( asio::error_code{}, 0 );
	}

	return sizeof( message_size ) + message_size;
}

bool TcpSocket::IsConnected() const {
	auto lock = AcquireLock();

	return m_impl->connected;
}

Endpoint TcpSocket::GetLocalEndpoint() const {
	auto lock = AcquireLock();

	IpAddress address;
	address.m_impl->address = m_impl->asio_socket.local_endpoint().address();

	return { address, m_impl->asio_socket.local_endpoint().port() };
}

Endpoint TcpSocket::GetRemoteEndpoint() const {
	auto lock = AcquireLock();

	IpAddress address;
	address.m_impl->address = m_impl->asio_socket.remote_endpoint().address();

	return { address, m_impl->asio_socket.remote_endpoint().port() };
}

void TcpSocket::ClearBuffers() {
	auto lock = AcquireLock();

	m_impl->send_buffer.clear();
	m_impl->receive_buffer.clear();

	m_impl->ReceiveHandler( asio::error_code{}, 0 );
}

std::size_t TcpSocket::BytesToSend() const {
	auto lock = AcquireLock();

	return m_impl->send_buffer.size();
}

std::size_t TcpSocket::BytesToReceive() const {
	auto lock = AcquireLock();

	return m_impl->receive_buffer.size();
}

int TcpSocket::GetLinger() const {
	auto lock = AcquireLock();

	asio::socket_base::linger option;

	asio::error_code error;

	m_impl->asio_socket.get_option( option, error );

	if( error ) {
		ErrorMessage() << "GetLinger() Error: " << error.message() << "\n";
	}

	return option.enabled() ? option.timeout() : 0;
}

void TcpSocket::SetLinger( int timeout ) {
	auto lock = AcquireLock();

	asio::socket_base::linger option( timeout > 0, timeout );

	asio::error_code error;

	m_impl->asio_socket.set_option( option, error );

	if( error ) {
		ErrorMessage() << "SetLinger() Error: " << error.message() << "\n";
	}
}

bool TcpSocket::GetKeepAlive() const {
	auto lock = AcquireLock();

	asio::socket_base::keep_alive option;

	asio::error_code error;

	m_impl->asio_socket.get_option( option, error );

	if( error ) {
		ErrorMessage() << "GetKeepAlive() Error: " << error.message() << "\n";
	}

	return option.value();
}

void TcpSocket::SetKeepAlive( bool keep_alive ) {
	auto lock = AcquireLock();

	asio::socket_base::keep_alive option( keep_alive );

	asio::error_code error;

	m_impl->asio_socket.set_option( option, error );

	if( error ) {
		ErrorMessage() << "SetKeepAlive() Error: " << error.message() << "\n";
	}
}

/// @cond
void TcpSocket::SetInternalSocket( void* internal_socket ) {
	m_impl->asio_socket = std::move( *static_cast<asio::ip::tcp::socket*>( internal_socket ) );

	OnConnected();

	m_impl->connected = true;
	m_impl->request_shutdown = false;
	m_impl->fin_sent = false;
	m_impl->fin_received = false;

	m_impl->ReceiveHandler( asio::error_code{}, 0 );
	m_impl->SendHandler( asio::error_code{}, 0 );
}
/// @endcond

}
