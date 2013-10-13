/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <iostream>
#include <SFNUL.hpp>

int main() {
	// Resolve our hostname to an address.
	auto addresses = sfn::IpAddress::Resolve( "www.ietf.org" );

	// Check if the name resolution was unsuccessful.
	if( addresses.empty() ) {
		std::cout << "Could not resolve hostname \"www.ietf.org\" to an address.\n";
		return 1;
	}

	// Create our TCP socket.
	auto socket = sfn::TcpSocket::Create();

	// Connect the TCP socket to the endpoint.
	socket->Connect( sfn::Endpoint{ addresses.front(), 80 } );

	// Construct our HTTP request.
	std::string request( "GET / HTTP/1.0\r\nHost: www.ietf.org\r\n\r\n" );

	// Send our HTTP request.
	socket->Send( request.c_str(), request.size() );

	// Start a network processing thread.
	sfn::Start();

	// Keep waiting until the remote has signalled that it has nothing more to send.
	while( !socket->RemoteHasShutdown() ) {
	}

	std::array<char, 1024> reply;
	std::size_t reply_size = 0;

	do {
		// Dequeue any data we receive from the remote host.
		reply_size = socket->Receive( reply.data(), reply.size() );

		// Print out the data.
		for( std::size_t index = 0; index < reply_size; index++ ) {
			std::cout << reply[index];
		}
	} while( reply_size );

	// Shutdown our side.
	socket->Shutdown();

	// Close the socket.
	socket->Close();

	// Stop all network processing threads.
	sfn::Stop();

	return 0;
}
