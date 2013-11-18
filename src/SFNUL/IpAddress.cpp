/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <iostream>
#include <SFNUL/Config.hpp>
#include <SFNUL/Utility.hpp>
#include <asio/ip/icmp.hpp>
#include <SFNUL/NetworkResource.hpp>
#include <SFNUL/IpAddress.hpp>

namespace sfn {

IpAddress::IpAddress() {
}

/// @cond
IpAddress::IpAddress( const asio::ip::address& asio_address ) :
	asio::ip::address{ asio_address }
{
}
/// @endcond

IpAddress::IpAddress( std::string address_string ) {
	asio::error_code error;

	( *this ) = from_string( address_string, error );

	if( error ) {
		ErrorMessage() << "IpAddress() Error: " << error.message() << "\n";
	}
}

IpAddress::~IpAddress() {
}

IpAddress::operator std::string() const {
	asio::error_code error;

	std::string address_string{ to_string( error ) };

	if( error ) {
		ErrorMessage() << "Error converting IpAddress to string: " << error.message() << "\n";

		return std::string();
	}

	return address_string;
}

bool IpAddress::IsIPv4() const {
	return is_v4();
}

bool IpAddress::IsIPv6() const {
	return is_v6();
}

std::deque<IpAddress> IpAddress::Resolve( const std::string& hostname ) {
	NetworkResource resource;

	asio::ip::icmp::resolver resolver{ resource.GetIOService() };
	asio::ip::icmp::resolver::query query{ hostname, "" };

	asio::error_code error;

	auto endpoint_iterator = resolver.resolve( query, error );

	std::deque<IpAddress> addresses{};

	if( error ) {
		ErrorMessage() << "Error resolving \"" << hostname << "\": " << error.message() << "\n";

		return addresses;
	}

	for( ; endpoint_iterator != asio::ip::icmp::resolver::iterator{}; ++endpoint_iterator ) {
		addresses.emplace_back( endpoint_iterator->endpoint().address() );
	}

	return addresses;
}

}
