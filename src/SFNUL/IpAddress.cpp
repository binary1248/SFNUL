/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <SFNUL/IpAddress.hpp>
#include <SFNUL/ConfigInternal.hpp>
#include <SFNUL/Utility.hpp>
#include <SFNUL/NetworkResource.hpp>
#include <SFNUL/IpAddressImpl.hpp>
#include <SFNUL/MakeUnique.hpp>
#include <asio/ip/icmp.hpp>
#include <asio/ip/address.hpp>

namespace sfn {

IpAddress::IpAddress() :
	m_impl{ make_unique<IpAddressImpl>() }
{
}

IpAddress::IpAddress( std::string address_string ) :
	m_impl{ make_unique<IpAddressImpl>() }
{
	asio::error_code error;

	m_impl->address = asio::ip::address::from_string( address_string, error );

	if( error ) {
		ErrorMessage() << "IpAddress() Error: " << error.message() << "\n";
	}
}

IpAddress::IpAddress( const IpAddress& address ) :
	m_impl{ make_unique<IpAddressImpl>() }
{
	m_impl->address = address.m_impl->address;
}

IpAddress::~IpAddress() = default;

IpAddress& IpAddress::operator=( const IpAddress& address ) {
	m_impl->address = address.m_impl->address;

	return *this;
}

bool IpAddress::operator==( const IpAddress& address ) const {
	return ( m_impl->address == address.m_impl->address );
}

IpAddress::operator std::string() const {
	asio::error_code error;

	std::string address_string{ m_impl->address.to_string( error ) };

	if( error ) {
		ErrorMessage() << "Error converting IpAddress to string: " << error.message() << "\n";

		return std::string();
	}

	return address_string;
}

bool IpAddress::IsIPv4() const {
	return m_impl->address.is_v4();
}

bool IpAddress::IsIPv6() const {
	return m_impl->address.is_v6();
}

std::deque<IpAddress> IpAddress::Resolve( const std::string& hostname ) {
	NetworkResource resource;

	asio::ip::icmp::resolver resolver{ *static_cast<asio::io_service*>( resource.GetIOService() ) };
	asio::ip::icmp::resolver::query query{ hostname, "" };

	asio::error_code error;

	auto endpoint_iterator = resolver.resolve( query, error );

	std::deque<IpAddress> addresses{};

	if( error ) {
		ErrorMessage() << "Error resolving \"" << hostname << "\": " << error.message() << "\n";

		return addresses;
	}

	for( ; endpoint_iterator != asio::ip::icmp::resolver::iterator{}; ++endpoint_iterator ) {
		addresses.emplace_back();
		addresses.back().m_impl->address = endpoint_iterator->endpoint().address();
	}

	return addresses;
}

}
