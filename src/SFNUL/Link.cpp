/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <cassert>
#include <iostream>
#include <SFML/Network/Packet.hpp>
#include <SFNUL/Endpoint.hpp>
#include <SFNUL/Link.hpp>
#include <SFNUL/Synchronizer.hpp>
#include <SFNUL/Message.hpp>

namespace sfn {

namespace {
struct PacketAccessor : public sf::Packet {
	const void* Send( std::size_t& size ) { return onSend( size ); }
	void Receive( const void* data, std::size_t size ) { onReceive( data, size ); }
};
}

const LinkBase::segment_size_type LinkBase::m_maximum_segment_size = 65535;

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

void LinkBase::Send( const void* data, std::size_t size ) {
	Send( 0, data, size );
}

std::size_t LinkBase::Receive( void* data, std::size_t size ) {
	return Receive( 0, data, size );
}

void LinkBase::Send( sf::Packet& packet ) {
	Send( 0, packet );
}

std::size_t LinkBase::Receive( sf::Packet& packet ) {
	return Receive( 0, packet );
}

void LinkBase::Send( const Message& message ) {
	Send( 0, message );
}

std::size_t LinkBase::Receive( Message& message ) {
	return Receive( 0, message );
}

void LinkBase::Send( stream_id_type stream_id, const void* data, std::size_t size ) {
	std::size_t sent = 0;

	// Split data up into segments.
	while( size ) {
		// Send stream id.
		GetInternalTransport()->Send( &stream_id, sizeof( stream_id ) );

		auto segment_size = static_cast<segment_size_type>( std::min( static_cast<std::size_t>( m_maximum_segment_size ), size ) );

		// Send segment size.
		GetInternalTransport()->Send( &segment_size, sizeof( segment_size ) );

		// Send segment data.
		GetInternalTransport()->Send( static_cast<const char*>( data ) + sent, segment_size );

		sent += segment_size;
		size -= segment_size;
	}
}

std::size_t LinkBase::Receive( stream_id_type stream_id, void* data, std::size_t size ) {
	if( m_segment_active && ( m_current_stream_reader != stream_id ) ) {
		return 0;
	}

	if( !m_segment_active ) {
		if( GetInternalTransport()->BytesToReceive() < sizeof( m_current_stream_reader ) + sizeof( m_segment_remaining ) ) {
			return 0;
		}

		auto stream_id_received = GetInternalTransport()->Receive( &m_current_stream_reader, sizeof( m_current_stream_reader ) );
		assert( stream_id_received == sizeof( m_current_stream_reader ) );

		auto segment_remaining_received = GetInternalTransport()->Receive( &m_segment_remaining, sizeof( m_segment_remaining ) );
		assert( segment_remaining_received == sizeof( m_segment_remaining ) );

		m_segment_active = true;

		if( m_current_stream_reader != stream_id ) {
			return 0;
		}
	}

	auto result = GetInternalTransport()->Receive( data, std::min( size, static_cast<std::size_t>( m_segment_remaining ) ) );

	assert( result <= m_maximum_segment_size );

	m_segment_remaining -= static_cast<segment_size_type>( result );

	if( !m_segment_remaining ) {
		m_segment_active = false;
	}

	return result;
}

void LinkBase::Send( stream_id_type stream_id, sf::Packet& packet ) {
	std::size_t size = 0;
	auto data = static_cast<PacketAccessor*>( &packet )->Send( size );

	sf::Uint32 packet_size = htonl( static_cast<sf::Uint32>( size ) );

	std::vector<char> data_block( sizeof( packet_size ) + size );

	std::memcpy( &data_block[0], &packet_size, sizeof( packet_size ) );

	if( size ) {
		std::memcpy( &data_block[0] + sizeof( packet_size ), data, size );
	}

	if( sizeof( packet_size ) + size > m_maximum_segment_size ) {
		std::cerr << "sf::Packets larger than " << m_maximum_segment_size - sizeof( packet_size ) << " bytes unsupported.\n";
		return;
	}

	Send( stream_id, &data_block[0], sizeof( packet_size ) + size );
}

std::size_t LinkBase::Receive( stream_id_type stream_id, sf::Packet& packet ) {
	if( m_segment_active && ( m_current_stream_reader != stream_id ) ) {
		return 0;
	}

	if( !m_segment_active ) {
		if( GetInternalTransport()->BytesToReceive() < sizeof( m_current_stream_reader ) + sizeof( m_segment_remaining ) ) {
			return 0;
		}

		auto stream_id_received = GetInternalTransport()->Receive( &m_current_stream_reader, sizeof( m_current_stream_reader ) );
		assert( stream_id_received == sizeof( m_current_stream_reader ) );

		auto segment_remaining_received = GetInternalTransport()->Receive( &m_segment_remaining, sizeof( m_segment_remaining ) );
		assert( segment_remaining_received == sizeof( m_segment_remaining ) );

		m_segment_active = true;

		if( m_current_stream_reader != stream_id ) {
			return 0;
		}
	}

	if( GetInternalTransport()->BytesToReceive() < m_segment_remaining ) {
		return 0;
	}

	auto result = GetInternalTransport()->Receive( packet );

	assert( result <= m_maximum_segment_size );
	assert( result == m_segment_remaining );

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

		auto stream_id_received = GetInternalTransport()->Receive( &m_current_stream_reader, sizeof( m_current_stream_reader ) );
		assert( stream_id_received == sizeof( m_current_stream_reader ) );

		auto segment_remaining_received = GetInternalTransport()->Receive( &m_segment_remaining, sizeof( m_segment_remaining ) );
		assert( segment_remaining_received == sizeof( m_segment_remaining ) );

		m_segment_active = true;

		if( m_current_stream_reader != stream_id ) {
			return 0;
		}
	}

	if( GetInternalTransport()->BytesToReceive() < m_segment_remaining ) {
		return 0;
	}

	auto result = GetInternalTransport()->Receive( message );

	assert( result <= m_maximum_segment_size );
	assert( result == m_segment_remaining );

	m_segment_remaining -= static_cast<segment_size_type>( result );

	if( !m_segment_remaining ) {
		m_segment_active = false;
	}

	return result;
}

void LinkBase::Send( stream_id_type stream_id, const Message& message ) {
	auto message_size = message.GetSize();

	std::vector<char> data_block( sizeof( message_size ) + message_size );

	std::memcpy( &data_block[0], &message_size, sizeof( message_size ) );

	if( message_size ) {
		const auto& message_data = message.GetBuffer();
		std::copy( std::begin( message_data ), std::end( message_data ), std::begin( data_block ) + sizeof( message_size ) );
	}

	if( sizeof( message_size ) + message_size > m_maximum_segment_size ) {
		std::cerr << "Messages larger than " << m_maximum_segment_size - sizeof( message_size ) << " bytes unsupported.\n";
		return;
	}

	Send( stream_id, &data_block[0], sizeof( message_size ) + static_cast<std::size_t>( message_size ) );
}

void LinkBase::SetInternalSocket( void* /*internal_socket*/ ) {
}

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
