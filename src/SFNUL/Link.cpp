/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <SFNUL/Link.hpp>
#include <SFNUL/Endpoint.hpp>
#include <SFNUL/Synchronizer.hpp>
#include <SFNUL/Message.hpp>
#include <cassert>

namespace sfn {

void LinkBase::Connect( const Endpoint& endpoint ) {
	GetInternalTransport()->Connect( endpoint );
}

void LinkBase::Shutdown() {
	GetInternalTransport()->Shutdown();
}

bool LinkBase::LocalHasShutdown() const {
	return GetInternalTransport()->LocalHasShutdown();
}

bool LinkBase::RemoteHasShutdown() const {
	return GetInternalTransport()->RemoteHasShutdown();
}

bool LinkBase::IsConnected() const {
	return GetInternalTransport()->IsConnected();
}

void LinkBase::Close() {
	GetInternalTransport()->Close();

	m_segment_active = false;
}

Endpoint LinkBase::GetLocalEndpoint() const {
	return GetInternalTransport()->GetLocalEndpoint();
}

Endpoint LinkBase::GetRemoteEndpoint() const {
	return GetInternalTransport()->GetRemoteEndpoint();
}

bool LinkBase::Send( const void* data, std::size_t size ) {
	return Send( 0, data, size );
}

std::size_t LinkBase::Receive( void* data, std::size_t size ) {
	return Receive( 0, data, size );
}

bool LinkBase::Send( const Message& message ) {
	return Send( 0, message );
}

std::size_t LinkBase::Receive( Message& message ) {
	return Receive( 0, message );
}

bool LinkBase::Send( stream_id_type stream_id, const void* data, std::size_t size ) {
	auto segment_size = static_cast<segment_size_type>( size );

	std::vector<char> data_block( size + sizeof( stream_id ) + sizeof( segment_size ) );

	// Send stream id.
	std::memcpy( data_block.data(), &stream_id, sizeof( stream_id ) );

	// Send segment size.
	std::memcpy( data_block.data() + sizeof( stream_id ), &segment_size, sizeof( segment_size ) );

	// Send segment data.
	std::memcpy( data_block.data() + sizeof( stream_id ) + sizeof( segment_size ), data, size );

	return GetInternalTransport()->Send( data_block.data(), data_block.size() );
}

std::size_t LinkBase::Receive( stream_id_type stream_id, void* data, std::size_t size ) {
	if( m_segment_active && ( m_current_stream_reader != stream_id ) ) {
		return 0;
	}

	if( !m_segment_active ) {
		if( GetInternalTransport()->BytesToReceive() < sizeof( m_current_stream_reader ) + sizeof( m_segment_remaining ) ) {
			return 0;
		}

#if defined( SFNUL_DEBUG )
		auto stream_id_received = GetInternalTransport()->Receive( &m_current_stream_reader, sizeof( m_current_stream_reader ) );
		assert( stream_id_received == sizeof( m_current_stream_reader ) );

		auto segment_remaining_received = GetInternalTransport()->Receive( &m_segment_remaining, sizeof( m_segment_remaining ) );
		assert( segment_remaining_received == sizeof( m_segment_remaining ) );
#else
		GetInternalTransport()->Receive( &m_current_stream_reader, sizeof( m_current_stream_reader ) );
		GetInternalTransport()->Receive( &m_segment_remaining, sizeof( m_segment_remaining ) );
#endif

		m_segment_active = true;

		if( m_current_stream_reader != stream_id ) {
			return 0;
		}
	}

	auto result = GetInternalTransport()->Receive( data, std::min( size, static_cast<std::size_t>( m_segment_remaining ) ) );

	m_segment_remaining -= static_cast<segment_size_type>( result );

	if( !m_segment_remaining ) {
		m_segment_active = false;
	}

	return result;
}

std::size_t LinkBase::Receive( stream_id_type stream_id, Message& message ) {
	if( m_segment_active && ( m_current_stream_reader != stream_id ) ) {
		return 0;
	}

	if( !m_segment_active ) {
		if( GetInternalTransport()->BytesToReceive() < sizeof( m_current_stream_reader ) + sizeof( m_segment_remaining ) ) {
			return 0;
		}

#if defined( SFNUL_DEBUG )
		auto stream_id_received = GetInternalTransport()->Receive( &m_current_stream_reader, sizeof( m_current_stream_reader ) );
		assert( stream_id_received == sizeof( m_current_stream_reader ) );

		auto segment_remaining_received = GetInternalTransport()->Receive( &m_segment_remaining, sizeof( m_segment_remaining ) );
		assert( segment_remaining_received == sizeof( m_segment_remaining ) );
#else
		GetInternalTransport()->Receive( &m_current_stream_reader, sizeof( m_current_stream_reader ) );
		GetInternalTransport()->Receive( &m_segment_remaining, sizeof( m_segment_remaining ) );
#endif

		m_segment_active = true;

		if( m_current_stream_reader != stream_id ) {
			return 0;
		}
	}

	if( GetInternalTransport()->BytesToReceive() < m_segment_remaining ) {
		return 0;
	}

	auto result = GetInternalTransport()->Receive( message );

	assert( result == m_segment_remaining );

	m_segment_remaining -= static_cast<segment_size_type>( result );

	if( !m_segment_remaining ) {
		m_segment_active = false;
	}

	return result;
}

bool LinkBase::Send( stream_id_type stream_id, const Message& message ) {
	auto message_size = message.GetSize();

	std::vector<char> data_block( sizeof( message_size ) + message_size );

	std::memcpy( &data_block[0], &message_size, sizeof( message_size ) );

	if( message_size ) {
		const auto& message_data = message.GetBuffer();
		std::copy( std::begin( message_data ), std::end( message_data ), std::begin( data_block ) + sizeof( message_size ) );
	}

	return Send( stream_id, &data_block[0], sizeof( message_size ) + static_cast<std::size_t>( message_size ) );
}

/// @cond
void LinkBase::SetInternalSocket( void* /*internal_socket*/ ) {
}
/// @endcond

void LinkBase::ClearBuffers() {
	return GetInternalTransport()->ClearBuffers();
}

std::size_t LinkBase::BytesToSend() const {
	return GetInternalTransport()->BytesToSend();
}

std::size_t LinkBase::BytesToReceive() const {
	return GetInternalTransport()->BytesToReceive();
}

}
