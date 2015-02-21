/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <SFNUL/Config.hpp>
#include <memory>
#include <deque>
#include <string>

namespace sfn {

class Endpoint;
class TcpListener;
class TcpSocket;
class UdpSocket;

/** An IP address. Can represent IPv4 or IPv6 addresses.
 */
class SFNUL_API IpAddress {

public:
	/** Constructor.
	 */
	IpAddress();

	/** Constructor.
	 * @param address_string String representation of an IPv4 or IPv6 address.
	 */
	IpAddress( std::string address_string );

	/** Copy Constructor.
	 * @param address IpAddress.
	 */
	IpAddress( const IpAddress& address );

	/** Destructor.
	 */
	~IpAddress();

	/** Assignment operator.
	 * @param address IpAddress.
	 */
	IpAddress& operator=( const IpAddress& address );

	/** Equality operator.
	 * @param address IpAddress.
	 * @return true if equal.
	 */
	bool operator==( const IpAddress& address ) const;

	/** Cast operator to std::string.
	 * Casts this IpAddress into its std::string representation.
	 */
	operator std::string() const;

	/** Check if the address is an IPv4 address.
	 * @return true if the address is an IPv4 address.
	 */
	bool IsIPv4() const;

	/** Check if the address is an IPv6 address.
	 * @return true if the address is an IPv6 address.
	 */
	bool IsIPv6() const;

	/** Resolves a hostname to a std::deque<IpAddress> containing all addresses the hostname identifies.
	 * @param hostname Hostname to resolve.
	 * @return std::deque<IpAddress> containing all addresses the hostname identifies.
	 */
	static std::deque<IpAddress> Resolve( const std::string& hostname );

private:
	class IpAddressImpl;
	std::unique_ptr<IpAddressImpl> m_impl;

	friend Endpoint;
	friend TcpListener;
	friend TcpSocket;
	friend UdpSocket;
};

}
