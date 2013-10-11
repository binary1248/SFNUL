/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <SFNUL/Config.hpp>
#include <asio/ip/basic_endpoint.hpp>
#include <SFNUL/IpAddress.hpp>

namespace sfn {

/** A class that uniquely describes an endpoint on a system when used with a specific transport protocol.
 */
class SFNUL_API Endpoint {
public:
	/** Constructor.
	 */
	Endpoint();

	/** Constructor.
	 * @param address IP address of the endpoint.
	 * @param port Port number of the endpoint.
	 */
	Endpoint( const IpAddress& address, unsigned short port );

	/** Constructor.
	 * This will construct an endpoint that describes all addresses and the given port on a system i.e. 0.0.0.0:port.
	 * @param port Port number of the endpoint.
	 */
	Endpoint( unsigned short port );

	/** Destructor.
	 */
	~Endpoint();

	/** Get the IP address associated with this endpoint.
	 * @return IP address associated with this endpoint.
	 */
	IpAddress GetAddress() const;

	/** Set the IP address associated with this endpoint.
	 * @param address IP address to associate with this endpoint.
	 */
	void SetAddress( const IpAddress& address );

	/** Get the port number associated with this endpoint.
	 * @return Port number associated with this endpoint.
	 */
	unsigned short GetPort() const;

	/** Set the port number associated with this endpoint.
	 * @param port Port number to associate with this endpoint.
	 */
	void SetPort( unsigned short port );

private:
	/** Construct an endpoint from an asio basic_endpoint<T>.
	 * @param endpoint asio basic_endpoint<T>
	 */
	template<typename T>
	Endpoint( asio::ip::basic_endpoint<T> endpoint );

	/** Convert to asio basic_endpoint<T>.
	 * @return asio basic_endpoint<T>
	 */
	template<typename T>
	asio::ip::basic_endpoint<T> GetInternalEndpoint() const;

	IpAddress m_address;
	unsigned short m_port = 0;

	friend class TcpListener;
	friend class TcpSocket;
	friend class UdpSocket;
};

template<typename T>
Endpoint::Endpoint( asio::ip::basic_endpoint<T> endpoint ) :
	m_address{ endpoint.address() },
	m_port{ endpoint.port() }
{
}

template<typename T>
asio::ip::basic_endpoint<T> Endpoint::GetInternalEndpoint() const {
	return asio::ip::basic_endpoint<T>{ m_address, m_port };
}

bool operator==( const Endpoint& left, const Endpoint& right );

}
