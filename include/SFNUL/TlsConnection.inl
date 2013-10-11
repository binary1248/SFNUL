/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <cstring>
#include <iostream>
#include <algorithm>
#include <SFML/Network/Packet.hpp>
#include <SFML/System/Lock.hpp>
#include <SFNUL/Message.hpp>

#if defined( SFNUL_SYSTEM_WINDOWS )
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

#if defined( NONE )
#undef NONE
#endif

#if defined( OPTIONAL )
#undef OPTIONAL
#endif

#if defined( REQUIRED )
#undef REQUIRED
#endif

namespace sfn {

namespace {
template<class T, TlsEndpointType U, TlsVerificationType V>
struct TlsConnectionMaker : public TlsConnection<T, U, V> {};

struct PacketAccessor : public sf::Packet {
	const void* Send( std::size_t& size ) { return onSend( size ); }
	void Receive( const void* data, std::size_t size ) { onReceive( data, size ); }
};

}

template<class T, TlsEndpointType U, TlsVerificationType V>
TlsConnection<T, U, V>::TlsConnection() {
	sfn::detail::ssl_set_endpoint( &m_ssl_context, ( m_type == TlsEndpointType::CLIENT ? SSL_IS_CLIENT : SSL_IS_SERVER ) );

	if( m_verify == TlsVerificationType::NONE ) {
		sfn::detail::ssl_set_authmode( &m_ssl_context, SSL_VERIFY_NONE );
	}
	else if( m_verify == TlsVerificationType::OPTIONAL ) {
		sfn::detail::ssl_set_authmode( &m_ssl_context, SSL_VERIFY_OPTIONAL );
	}
	else if( m_verify == TlsVerificationType::REQUIRED ) {
		sfn::detail::ssl_set_authmode( &m_ssl_context, SSL_VERIFY_REQUIRED );
	}

	sfn::detail::ssl_set_bio(
		&m_ssl_context,
		[]( void* context, unsigned char* buffer, int length ) {
			return static_cast<TlsConnection<T, U, V>*>( context )->RecvInterface( nullptr, buffer, length );
		},
		this,
		[]( void* context, const unsigned char* buffer, int length ) {
			return static_cast<TlsConnection<T, U, V>*>( context )->SendInterface( nullptr, buffer, length );
		},
		this
	);
}

template<class T, TlsEndpointType U, TlsVerificationType V>
TlsConnection<T, U, V>::~TlsConnection() {
	Close();
}

template<class T, TlsEndpointType U, TlsVerificationType V>
typename TlsConnection<T, U, V>::Ptr TlsConnection<T, U, V>::Create() {
	return std::make_shared<TlsConnectionMaker<T, U, V>>();
}

template<class T, TlsEndpointType U, TlsVerificationType V>
int TlsConnection<T, U, V>::SendInterface( void* /*unused*/, const unsigned char* buffer, int length ) {
	T::Send( buffer, length );

	return length;
}

template<class T, TlsEndpointType U, TlsVerificationType V>
int TlsConnection<T, U, V>::RecvInterface( void* /*unused*/, unsigned char* buffer, int length ) {
	std::size_t received = 0;

	received = T::Receive( buffer, length );

	if( received ) {
		return received;
	}

	return TROPICSSL_ERR_NET_TRY_AGAIN;
}

template<class T, TlsEndpointType U, TlsVerificationType V>
void TlsConnection<T, U, V>::Connect( const Endpoint& endpoint ) {
	T::Connect( endpoint );
}

template<class T, TlsEndpointType U, TlsVerificationType V>
void TlsConnection<T, U, V>::Shutdown() {
	sf::Lock lock{ T::m_mutex };

	if( m_local_closed || T::LocalHasShutdown() || !T::IsConnected() ) {
		return;
	}

	m_request_close = true;

	if( !m_send_buffer.empty() ) {
		OnSent();
		return;
	}

	int result = sfn::detail::ssl_close_notify( &m_ssl_context );

	if( result ) {
		std::cerr << "TlsConnection::Shutdown() Error: ssl_close_notify returned: " << result << "\n";
		return;
	}

	m_local_closed = true;

	T::Shutdown();
}

template<class T, TlsEndpointType U, TlsVerificationType V>
bool TlsConnection<T, U, V>::LocalHasShutdown() const {
	return m_local_closed && T::LocalHasShutdown();
}

template<class T, TlsEndpointType U, TlsVerificationType V>
bool TlsConnection<T, U, V>::RemoteHasShutdown() const {
	return T::RemoteHasShutdown() || m_remote_closed;
}

template<class T, TlsEndpointType U, TlsVerificationType V>
bool TlsConnection<T, U, V>::IsConnected() const {
	return T::IsConnected() && !m_local_closed && !m_remote_closed;
}

template<class T, TlsEndpointType U, TlsVerificationType V>
void TlsConnection<T, U, V>::Close() {
	sf::Lock lock{ T::m_mutex };

	if( !m_local_closed ) {
		if( !m_send_buffer.empty() ) {
			std::cerr << "TlsConnection::Close(): Warning, did not send all data before shutdown, possible data loss might occur.\n";
		}
	}

	T::Close();
}

/// @cond
template<class T, TlsEndpointType U, TlsVerificationType V>
void TlsConnection<T, U, V>::Reset() {
	T::Reset();
}
/// @endcond

template<class T, TlsEndpointType U, TlsVerificationType V>
void TlsConnection<T, U, V>::Send( const void* data, std::size_t size ) {
	if( !data || !size ) {
		return;
	}

	{
		sf::Lock lock{ T::m_mutex };

		m_send_buffer.insert( m_send_buffer.end(), static_cast<const char*>( data ), static_cast<const char*>( data ) + size );
	}

	OnSent();
}

template<class T, TlsEndpointType U, TlsVerificationType V>
std::size_t TlsConnection<T, U, V>::Receive( void* data, std::size_t size ) {
	OnReceived();

	if( !data || !size ) {
		return 0;
	}

	sf::Lock lock{ T::m_mutex };

	auto receive_size = std::min( size, m_receive_buffer.size() );

	for( std::size_t index = 0; index < receive_size; index++ ) {
		static_cast<char*>( data )[index] = m_receive_buffer[index];
	}

	m_receive_buffer.erase( m_receive_buffer.begin(), m_receive_buffer.begin() + receive_size );

	return receive_size;
}

template<class T, TlsEndpointType U, TlsVerificationType V>
void TlsConnection<T, U, V>::Send( sf::Packet& packet ) {
	sf::Lock lock{ T::m_mutex };

	std::size_t size = 0;
	auto data = static_cast<PacketAccessor*>( &packet )->Send( size );

	sf::Uint32 packet_size = htonl( static_cast<sf::Uint32>( size ) );

	std::vector<char> data_block( sizeof( packet_size ) + size );

	std::memcpy( &data_block[0], &packet_size, sizeof( packet_size ) );

	if( size ) {
		std::memcpy( &data_block[0] + sizeof( packet_size ), data, size );
	}

	Send( &data_block[0], sizeof( packet_size ) + size );
}

template<class T, TlsEndpointType U, TlsVerificationType V>
std::size_t TlsConnection<T, U, V>::Receive( sf::Packet& packet ) {
	sf::Lock lock{ T::m_mutex };

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

	m_receive_buffer.erase( m_receive_buffer.begin(), m_receive_buffer.begin() + packet_size );

	return sizeof( packet_size ) + packet_size;
}

template<class T, TlsEndpointType U, TlsVerificationType V>
void TlsConnection<T, U, V>::Send( const Message& message ) {
	sf::Lock lock{ T::m_mutex };

	auto message_size = message.GetSize();

	std::vector<char> data_block( sizeof( message_size ) + message_size );

	std::memcpy( &data_block[0], &message_size, sizeof( message_size ) );

	if( message_size ) {
		const auto& message_data = message.GetBuffer();
		std::copy( std::begin( message_data ), std::end( message_data ), std::begin( data_block ) + sizeof( message_size ) );
	}

	Send( &data_block[0], sizeof( message_size ) + static_cast<std::size_t>( message_size ) );
}

template<class T, TlsEndpointType U, TlsVerificationType V>
std::size_t TlsConnection<T, U, V>::Receive( Message& message ) {
	sf::Lock lock{ T::m_mutex };

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

	std::vector<char> data_block( message_size );

	m_receive_buffer.erase( m_receive_buffer.begin(), m_receive_buffer.begin() + sizeof( message_size ) );

	std::copy_n( m_receive_buffer.begin(), message_size, data_block.begin() );

	message.Append( data_block );

	m_receive_buffer.erase( m_receive_buffer.begin(), m_receive_buffer.begin() + message_size );

	return sizeof( message_size ) + message_size;
}

template<class T, TlsEndpointType U, TlsVerificationType V>
void TlsConnection<T, U, V>::ClearBuffers() {
	sf::Lock lock{ T::m_mutex };

	m_send_buffer.clear();
	m_receive_buffer.clear();
}

template<class T, TlsEndpointType U, TlsVerificationType V>
std::size_t TlsConnection<T, U, V>::BytesToSend() const {
	sf::Lock lock{ T::m_mutex };

	return m_send_buffer.size();
}

template<class T, TlsEndpointType U, TlsVerificationType V>
std::size_t TlsConnection<T, U, V>::BytesToReceive() const {
	sf::Lock lock{ T::m_mutex };

	return m_receive_buffer.size();
}

template<class T, TlsEndpointType U, TlsVerificationType V>
void TlsConnection<T, U, V>::OnSent() {
	sf::Lock lock{ T::m_mutex };

	if( m_local_closed ) {
		return;
	}

	auto send_size = std::min( m_send_memory.size(), m_send_buffer.size() );

	if( !send_size && ( m_ssl_context.state == SSL_HANDSHAKE_OVER ) ) {
		return;
	}

	std::copy_n( m_send_buffer.begin(), send_size, m_send_memory.begin() );

	int length = 0;
	std::size_t current_location = 0;
	bool start = false;

	do {
		auto state_before = m_ssl_context.state;

		length = sfn::detail::ssl_write( &m_ssl_context, reinterpret_cast<const unsigned char*>( m_send_memory.data() + current_location ), send_size );

		if( !start ) {
			start = ( state_before != SSL_HANDSHAKE_OVER ) && ( m_ssl_context.state == SSL_HANDSHAKE_OVER );
		}

		if( length > 0 ) {
			current_location += length;
			send_size -= length;
		}
	} while( length > 0 );

	if( current_location ) {
		m_send_buffer.erase( m_send_buffer.begin(), m_send_buffer.begin() + current_location );
	}

	if( m_request_close && !m_local_closed && m_send_buffer.empty() ) {
		Shutdown();
	}

	if( length < 0 ) {
		if( length == TROPICSSL_ERR_NET_TRY_AGAIN ) {
		}
		else if( length == TROPICSSL_ERR_SSL_CERTIFICATE_REQUIRED ) {
			require_certificate_key = true;
		}
		else {
			std::cerr << "TlsConnection::OnSent() Error: ssl_write returned: " << length << "\n";
			return;
		}
	}

	if( start ) {
		OnReceived();
	}
}

template<class T, TlsEndpointType U, TlsVerificationType V>
void TlsConnection<T, U, V>::OnReceived() {
	sf::Lock lock{ T::m_mutex };

	if( m_remote_closed ) {
		return;
	}

	int length = 0;
	auto start = false;

	do {
		auto state_before = m_ssl_context.state;

		length = sfn::detail::ssl_read( &m_ssl_context, reinterpret_cast<unsigned char*>( m_receive_memory.data() ), m_receive_memory.size() );

		if( !start ) {
			start = ( state_before != SSL_HANDSHAKE_OVER ) && ( m_ssl_context.state == SSL_HANDSHAKE_OVER );
		}

		if( length > 0 ) {
			m_receive_buffer.insert( m_receive_buffer.end(), m_receive_memory.begin(), m_receive_memory.begin() + length );
		}
	} while( length > 0 );

	if( length < 0 ) {
		if( length == TROPICSSL_ERR_NET_TRY_AGAIN ) {
		}
		else if( length == TROPICSSL_ERR_SSL_PEER_CLOSE_NOTIFY ) {
			m_remote_closed = true;
		}
		else if( length == TROPICSSL_ERR_X509_CERT_VERIFY_FAILED ) {
			std::cerr << "Error: Verification of the peer certificate has failed. (specified REQUIRED)\n";
			return;
		}
		else if( length == TROPICSSL_ERR_SSL_CERTIFICATE_REQUIRED ) {
			require_certificate_key = true;
		}
		else {
			std::cerr << "TlsConnection::OnReceived() Error: ssl_read returned: " << length << "\n";
			return;
		}
	}

	if( start ) {
		OnSent();
	}
}

template<class T, TlsEndpointType U, TlsVerificationType V>
void TlsConnection<T, U, V>::OnConnected() {
	sf::Lock lock{ T::m_mutex };

	auto result = sfn::detail::ssl_handshake( &m_ssl_context );

	if( result && ( result != TROPICSSL_ERR_NET_TRY_AGAIN ) ) {
		std::cerr << "TlsConnection::OnConnected() Error: ssl_handshake returned: " << result << "\n";
		return;
	}

	m_local_closed = false;
	m_remote_closed = false;
}

template<class T, TlsEndpointType U, TlsVerificationType V>
void TlsConnection<T, U, V>::OnDisconnected() {
}

template<class T, TlsEndpointType U, TlsVerificationType V>
sf::Mutex& TlsConnection<T, U, V>::GetMutex() const {
	return T::m_mutex;
}

template<class T, TlsEndpointType U, TlsVerificationType V>
void TlsConnection<T, U, V>::OnSentProxy() {
	OnSent();
}

template<class T, TlsEndpointType U, TlsVerificationType V>
void TlsConnection<T, U, V>::OnReceivedProxy() {
	OnReceived();
}

}
