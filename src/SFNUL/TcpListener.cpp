/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <SFNUL/TcpListener.hpp>
#include <SFNUL/ConfigInternal.hpp>
#include <SFNUL/Utility.hpp>
#include <SFNUL/Endpoint.hpp>
#include <SFNUL/IpAddressImpl.hpp>
#include <SFNUL/MakeUnique.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/strand.hpp>
#include <functional>

namespace {

struct TcpListenerMaker : public sfn::TcpListener {};

}

namespace sfn {

class TcpListener::TcpListenerImpl {
public:
	TcpListenerImpl( TcpListener* owner ) :
		tcp_listener{ owner },
		asio_acceptor{ *static_cast<asio::io_service*>( owner->GetIOService() ) }
	{
	}

	void AcceptHandler( const asio::error_code& error, std::shared_ptr<asio::ip::tcp::socket> socket ) {
		auto lock = tcp_listener->AcquireLock();

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
			new_connections.emplace_back( std::move( *socket ) );
		}

		if( !listening ) {
			return;
		}

		auto new_socket = std::make_shared<asio::ip::tcp::socket>( *static_cast<asio::io_service*>( tcp_listener->GetIOService() ) );

		asio_acceptor.async_accept( *new_socket,
			static_cast<asio::strand*>( tcp_listener->GetStrand() )->wrap(
				std::bind(
					[]( std::weak_ptr<TcpListener> listener, const asio::error_code& handler_error, std::shared_ptr<asio::ip::tcp::socket> handler_socket ) {
						if( listener.expired() ) {
							return;
						}

						listener.lock()->m_impl->AcceptHandler( handler_error, handler_socket );
					},
					std::weak_ptr<TcpListener>( tcp_listener->shared_from_this() ), std::placeholders::_1, new_socket
				)
			)
		);
	}

	TcpListener* tcp_listener;

	asio::ip::tcp::acceptor asio_acceptor;

	std::deque<asio::ip::tcp::socket> new_connections;

	bool listening = false;
};

TcpListener::TcpListener() :
	m_impl{ make_unique<TcpListenerImpl>( this ) }
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

	auto asio_endpoint = asio::ip::basic_endpoint<asio::ip::tcp>{ endpoint.GetAddress().m_impl->address, endpoint.GetPort() };

	m_impl->asio_acceptor.open( asio_endpoint.protocol(), error );

	if( error ) {
		ErrorMessage() << "Listen() Error: " << error.message() << "\n";
		return;
	}

	asio::socket_base::enable_connection_aborted option( true );
	m_impl->asio_acceptor.set_option( option, error );

	if( error ) {
		ErrorMessage() << "Listen() Error: " << error.message() << "\n";
		return;
	}

	m_impl->asio_acceptor.bind( asio_endpoint, error );

	if( error ) {
		ErrorMessage() << "Listen() Error: " << error.message() << "\n";
		return;
	}

	m_impl->asio_acceptor.listen( backlog, error );

	if( error ) {
		ErrorMessage() << "Listen() Error: " << error.message() << "\n";
		return;
	}

	m_impl->listening = true;

	m_impl->AcceptHandler( asio::error_code{}, nullptr );
}

void TcpListener::Close() {
	auto lock = AcquireLock();

	asio::error_code error;

	m_impl->asio_acceptor.close( error );

	if( error ) {
		ErrorMessage() << "Close() Error: " << error.message() << "\n";
		return;
	}

	m_impl->listening = false;
}

Endpoint TcpListener::GetEndpoint() const {
	auto lock = AcquireLock();

	IpAddress address;
	address.m_impl->address = m_impl->asio_acceptor.local_endpoint().address();

	return { address, m_impl->asio_acceptor.local_endpoint().port() };
}

bool TcpListener::HasPendingConnections() const {
	return !m_impl->new_connections.empty();
}

void* TcpListener::GetFirstPendingConnection() {
	return &( m_impl->new_connections.front() );
}

void TcpListener::RemoveFirstPendingConnection() {
	m_impl->new_connections.pop_front();
}

}
