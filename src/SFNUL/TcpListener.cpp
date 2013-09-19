#include <functional>
#include <iostream>
#include <SFNUL/Endpoint.hpp>
#include <SFNUL/TcpListener.hpp>

namespace sfn {
namespace {
struct TcpListenerMaker: public TcpListener {};
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
	sf::Lock lock{ m_mutex };

	asio::error_code error;

	auto asio_endpoint = endpoint.GetInternalEndpoint<asio::ip::tcp>();

	m_acceptor.open( asio_endpoint.protocol(), error );

	if( error ) {
		std::cerr << "Listen() Error: " << error.message() << "\n";
		return;
	}

	asio::socket_base::enable_connection_aborted option( true );
	m_acceptor.set_option( option, error );

	if( error ) {
		std::cerr << "Listen() Error: " << error.message() << "\n";
		return;
	}

	m_acceptor.bind( asio_endpoint, error );

	if( error ) {
		std::cerr << "Listen() Error: " << error.message() << "\n";
		return;
	}

	m_acceptor.listen( backlog, error );

	if( error ) {
		std::cerr << "Listen() Error: " << error.message() << "\n";
		return;
	}

	m_listening = true;

	AcceptHandler( asio::error_code{}, nullptr );
}

void TcpListener::AcceptHandler( const asio::error_code& error, std::shared_ptr<asio::ip::tcp::socket> socket ) {
	sf::Lock lock{ m_mutex };

	if( !socket ) {
	}
	else if( error == asio::error::operation_aborted ) {
	}
	else if( ( error == asio::error::connection_aborted ) || ( error == asio::error::connection_reset ) ) {
		socket.reset();
	}
	else if( error ) {
		std::cerr << "Async Accept Error: " << error.message() << "\n";
		return;
	}

	if( socket ) {
		if( m_new_connections.size() < m_connection_limit_hard ) {
			m_new_connections.push_back( std::move( *socket ) );
		}
		else {
			asio::error_code shutdown_error;

			socket->shutdown( asio::ip::tcp::socket::shutdown_both, shutdown_error );

			socket->close();

			std::cerr << "Async Accept Warning: Pending connection count (" << m_new_connections.size() << ") exceeds hard limit of " << m_connection_limit_hard << ". Dropping connection.\n";
		}
	}

	if( m_new_connections.size() > m_connection_limit_soft ) {
		std::cerr << "Async Accept Warning: Pending connection count (" << m_new_connections.size() << ") exceeds soft limit of " << m_connection_limit_soft << ".\n";
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
	sf::Lock lock{ m_mutex };

	asio::error_code error;

	m_acceptor.close( error );

	if( error ) {
		std::cerr << "Close() Error: " << error.message() << "\n";
		return;
	}

	m_listening = false;
}

Endpoint TcpListener::GetEndpoint() const {
	sf::Lock lock{ m_mutex };

	return m_acceptor.local_endpoint();
}

std::size_t TcpListener::ConnectionSoftLimit( std::size_t limit ) {
	sf::Lock lock{ m_mutex };

	return m_connection_limit_soft = ( limit ? limit : m_connection_limit_soft );
}

std::size_t TcpListener::ConnectionHardLimit( std::size_t limit ) {
	sf::Lock lock{ m_mutex };

	return m_connection_limit_hard = ( limit ? limit : m_connection_limit_hard );
}

}
