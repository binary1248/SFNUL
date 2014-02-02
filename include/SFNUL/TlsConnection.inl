/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <cstring>
#include <iostream>
#include <algorithm>
#include <SFNUL/Utility.hpp>
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

// Until C++14 comes along...
namespace std {
template<typename T, typename... Args>
inline std::unique_ptr<T> make_unique( Args&&... args ) {
	return std::unique_ptr<T>( new T( std::forward<Args>( args )... ) );
}
}

namespace sfn {

namespace {
template<class T, TlsEndpointType U, TlsVerificationType V>
struct TlsConnectionMaker : public TlsConnection<T, U, V> {};
}

template<class T, TlsEndpointType U, TlsVerificationType V>
TlsConnection<T, U, V>::TlsConnection() {
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
void TlsConnection<T, U, V>::OutputCallback( const unsigned char* buffer, std::size_t size ) {
	T::Send( buffer, size );
}

template<class T, TlsEndpointType U, TlsVerificationType V>
void TlsConnection<T, U, V>::DataCallback( const unsigned char* buffer, std::size_t size ) {
	auto lock = T::AcquireLock();

	if( size > 0 ) {
		m_receive_buffer.insert( m_receive_buffer.end(), buffer, buffer + size );
	}
}

template<class T, TlsEndpointType U, TlsVerificationType V>
void TlsConnection<T, U, V>::AlertCallback( Botan::TLS::Alert, const unsigned char*, size_t ) {
}

template<class T, TlsEndpointType U, TlsVerificationType V>
bool TlsConnection<T, U, V>::HandshakeCallback( const Botan::TLS::Session& ) {
	// No session caching for now...
	return false;
}

template<class T, TlsEndpointType U, TlsVerificationType V>
void TlsConnection<T, U, V>::Connect( const Endpoint& endpoint ) {
	T::Connect( endpoint );
}

template<class T, TlsEndpointType U, TlsVerificationType V>
void TlsConnection<T, U, V>::Shutdown() {
	auto lock = T::AcquireLock();

	if( m_local_closed || T::LocalHasShutdown() || !T::IsConnected() ) {
		return;
	}

	m_request_close = true;

	if( m_tls_endpoint && m_tls_endpoint->is_active() ) {
		m_tls_endpoint->close();

		m_local_closed = true;

		T::Shutdown();
	}
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
	auto lock = T::AcquireLock();

	if( !m_local_closed ) {
		if( !m_send_buffer.empty() ) {
			WarningMessage() << "TlsConnection::Close(): Warning, did not send all data before shutdown, possible data loss might occur.\n";
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
bool TlsConnection<T, U, V>::Send( const void* data, std::size_t size ) {
	if( !data || !size ) {
		return false;
	}

	{
		auto lock = T::AcquireLock();

		if( m_send_buffer.size() + size >= GetMaximumBlockSize() / 2 ) {
			return false;
		}

		if( m_tls_endpoint && m_tls_endpoint->is_active() ) {
			m_tls_endpoint->send( static_cast<const unsigned char*>( data ), size );
		}
		else {
			m_send_buffer.insert( m_send_buffer.end(), static_cast<const char*>( data ), static_cast<const char*>( data ) + size );
		}
	}

	return true;
}

template<class T, TlsEndpointType U, TlsVerificationType V>
std::size_t TlsConnection<T, U, V>::Receive( void* data, std::size_t size ) {
	OnReceived();

	if( !data || !size ) {
		return 0;
	}

	auto lock = T::AcquireLock();

	auto receive_size = std::min( size, m_receive_buffer.size() );

	for( std::size_t index = 0; index < receive_size; index++ ) {
		static_cast<char*>( data )[index] = m_receive_buffer[index];
	}

	m_receive_buffer.erase( m_receive_buffer.begin(), m_receive_buffer.begin() + static_cast<int>( receive_size ) );

	return receive_size;
}

template<class T, TlsEndpointType U, TlsVerificationType V>
bool TlsConnection<T, U, V>::Send( const Message& message ) {
	auto lock = T::AcquireLock();

	auto message_size = message.GetSize();

	if( ( m_send_buffer.size() + sizeof( message_size ) + message_size >= GetMaximumBlockSize() / 2 ) ||
			( T::BytesToSend() + sizeof( message_size ) + message_size >= GetMaximumBlockSize() / 2 ) ) {
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

template<class T, TlsEndpointType U, TlsVerificationType V>
std::size_t TlsConnection<T, U, V>::Receive( Message& message ) {
	auto lock = T::AcquireLock();

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
	auto lock = T::AcquireLock();

	m_send_buffer.clear();
	m_receive_buffer.clear();
}

template<class T, TlsEndpointType U, TlsVerificationType V>
std::size_t TlsConnection<T, U, V>::BytesToSend() const {
	return T::BytesToSend();
}

template<class T, TlsEndpointType U, TlsVerificationType V>
std::size_t TlsConnection<T, U, V>::BytesToReceive() const {
	auto lock = T::AcquireLock();

	return m_receive_buffer.size();
}

template<class T, TlsEndpointType U, TlsVerificationType V>
void TlsConnection<T, U, V>::OnReceived() {
	auto lock = T::AcquireLock();

	if( m_remote_closed ) {
		return;
	}

	std::size_t received = 0;

	while( true ) {
		received = T::Receive( m_receive_memory.data(), m_receive_memory.size() );

		if( received ) {
			m_tls_endpoint->received_data( reinterpret_cast<unsigned char*>( m_receive_memory.data() ), received );

			if( m_tls_endpoint->is_active() && ( m_last_verification_result != TlsVerificationResult::PASSED ) && ( m_verify == TlsVerificationType::REQUIRED ) ) {
				ErrorMessage() << "Certificate verification failed (specified REQUIRED), closing connection.\n";

				m_tls_endpoint->close();

				T::Shutdown();
			}
			else {
				if( m_tls_endpoint->is_active() && !m_send_buffer.empty() ) {
					m_tls_endpoint->send( reinterpret_cast<const unsigned char*>( m_send_buffer.data() ), m_send_buffer.size() );
					m_send_buffer.clear();
				}

				if( m_tls_endpoint->is_active() && m_request_close ) {
					m_tls_endpoint->close();

					m_local_closed = true;

					T::Shutdown();
				}
			}
		}
		else {
			break;
		}
	}
}

template<class T, TlsEndpointType U, TlsVerificationType V>
void TlsConnection<T, U, V>::OnConnected() {
	auto lock = T::AcquireLock();

	m_local_closed = false;
	m_remote_closed = false;

	if( m_type == TlsEndpointType::CLIENT ) {
		m_tls_endpoint = std::make_unique<Botan::TLS::Client>(
			std::bind( &TlsConnection<T, U, V>::OutputCallback, this, std::placeholders::_1, std::placeholders::_2 ),
			std::bind( &TlsConnection<T, U, V>::DataCallback, this, std::placeholders::_1, std::placeholders::_2 ),
			std::bind( &TlsConnection<T, U, V>::AlertCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 ),
			std::bind( &TlsConnection<T, U, V>::HandshakeCallback, this, std::placeholders::_1 ),
			m_tls_session_manager,
			*this, // credentials_manager
			m_tls_policy,
			m_rng,
			Botan::TLS::Server_Information(),
			Botan::TLS::Protocol_Version::latest_tls_version()
		);
	}
	else if( m_type == TlsEndpointType::SERVER ) {
		m_tls_endpoint = std::make_unique<Botan::TLS::Server>(
			std::bind( &TlsConnection<T, U, V>::OutputCallback, this, std::placeholders::_1, std::placeholders::_2 ),
			std::bind( &TlsConnection<T, U, V>::DataCallback, this, std::placeholders::_1, std::placeholders::_2 ),
			std::bind( &TlsConnection<T, U, V>::AlertCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 ),
			std::bind( &TlsConnection<T, U, V>::HandshakeCallback, this, std::placeholders::_1 ),
			m_tls_session_manager,
			*this, // credentials_manager
			m_tls_policy,
			m_rng
		);
	}
}

template<class T, TlsEndpointType U, TlsVerificationType V>
void TlsConnection<T, U, V>::OnDisconnected() {
	m_tls_endpoint.reset();
}

}
