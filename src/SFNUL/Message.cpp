/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <SFNUL/Message.hpp>

namespace sfn {

void Message::Clear() {
	m_data.clear();
}

void Message::Prepend( const char* data, std::size_t size ) {
	m_data.insert( std::begin( m_data ), data, data + size );
}

void Message::Prepend( const std::vector<char>& data ) {
	m_data.insert( std::begin( m_data ), std::begin( data ), std::end( data ) );
}

void Message::Append( const char* data, std::size_t size ) {
	m_data.insert( std::end( m_data ), data, data + size );
}

void Message::Append( const std::vector<char>& data ) {
	m_data.insert( std::end( m_data ), std::begin( data ), std::end( data ) );
}

Message::size_type Message::GetSize() const {
	return static_cast<size_type>( m_data.size() );
}

std::vector<char> Message::GetFront( std::size_t size ) {
	std::vector<char> data( size );

	std::copy( std::begin( m_data ), std::begin( m_data ) + size, std::begin( data ) );

	return data;
}

std::vector<char> Message::GetBack( std::size_t size ) {
	std::vector<char> data( size );

	std::copy( std::end( m_data ) - size, std::end( m_data ), std::begin( data ) );

	return data;
}

void Message::PopFront( std::size_t size ) {
	m_data.erase( std::begin( m_data ), std::begin( m_data ) + size );
}

void Message::PopBack( std::size_t size ) {
	m_data.erase( std::end( m_data ) - size, std::end( m_data ) );
}

const std::deque<unsigned char>& Message::GetBuffer() const {
	return m_data;
}

}
