/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <SFNUL/Config.hpp>
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
	IpAddress m_address;
	unsigned short m_port = 0;

	friend class TcpListener;
	friend class TcpSocket;
	friend class UdpSocket;
};

bool operator==( const Endpoint& left, const Endpoint& right );

}
