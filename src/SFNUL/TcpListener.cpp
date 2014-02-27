/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <functional>
#include <iostream>
#include <SFNUL/Utility.hpp>
#include <SFNUL/Endpoint.hpp>
#include <SFNUL/TcpListener.hpp>

namespace sfn {
namespace {

struct TcpListenerMaker : public TcpListener {};

}

TcpListener::TcpListener() :
	m_acceptor{ GetIOService() }
{
}

TcpListener::~TcpListener() {
}

TcpListener::Ptr TcpListener::Create() {
	return std::make_shared<TcpListenerMaker>();
}

void TcpListener::Listen( const Endpoint& endpoint, int backlog ) {
	auto lock = AcquireLock();

	asio::error_code error;

	auto asio_endpoint = endpoint.GetInternalEndpoint<asio::ip::tcp>();

	m_acceptor.open( asio_endpoint.protocol(), error );

	if( error ) {
		ErrorMessage() << "Listen() Error: " << error.message() << "\n";
		return;
	}

	asio::socket_base::enable_connection_aborted option( true );
	m_acceptor.set_option( option, error );

	if( error ) {
		ErrorMessage() << "Listen() Error: " << error.message() << "\n";
		return;
	}

	m_acceptor.bind( asio_endpoint, error );

	if( error ) {
		ErrorMessage() << "Listen() Error: " << error.message() << "\n";
		return;
	}

	m_acceptor.listen( backlog, error );

	if( error ) {
		ErrorMessage() << "Listen() Error: " << error.message() << "\n";
		return;
	}

	m_listening = true;

	AcceptHandler( asio::error_code{}, nullptr );
}

void TcpListener::AcceptHandler( const asio::error_code& error, std::shared_ptr<asio::ip::tcp::socket> socket ) {
	auto lock = AcquireLock();

	if( !socket ) {
	}
	else if( error == asio::error::operation_aborted ) {
	}
	else if( ( error == asio::error::connection_aborted ) || ( error == asio::error::connection_reset ) ) {
		socket.reset();
	}
	else if( error ) {
		ErrorMessage() << "Async Accept Error: " << error.message() << "\n";
		return;
	}

	if( socket ) {
		m_new_connections.emplace_back( std::move( *socket ) );
	}

	if( !m_listening ) {
		return;
	}

	auto new_socket = std::make_shared<asio::ip::tcp::socket>( GetIOService() );

	m_acceptor.async_accept( *new_socket,
		m_strand.wrap(
			std::bind(
				[]( std::weak_ptr<TcpListener> listener, const asio::error_code& handler_error, std::shared_ptr<asio::ip::tcp::socket> handler_socket ) {
					if( listener.expired() ) {
						return;
					}

					listener.lock()->AcceptHandler( handler_error, handler_socket );
				},
				std::weak_ptr<TcpListener>( shared_from_this() ), std::placeholders::_1, new_socket
			)
		)
	);
}

void TcpListener::Close() {
	auto lock = AcquireLock();

	asio::error_code error;

	m_acceptor.close( error );

	if( error ) {
		ErrorMessage() << "Close() Error: " << error.message() << "\n";
		return;
	}

	m_listening = false;
}

Endpoint TcpListener::GetEndpoint() const {
	auto lock = AcquireLock();

	return m_acceptor.local_endpoint();
}

}
