/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <iostream>
#include <SFNUL.hpp>

int main() {
	// Create our TCP listener socket.
	auto listener = sfn::TcpListener::Create();

	// Get our localhost address (if possible IPv6 address).
	sfn::IpAddress address;

	auto addresses = sfn::IpAddress::Resolve( "localhost" );

	for( auto a : addresses ) {
		address = a;

		if( address.IsIPv6() ) {
			// Congratulate the user for running a IPv6 capable system.
			std::cout << "Wow! This host supports IPv6 too!\n";
			break;
		}
	}

	// Listen on localhost:1337
	listener->Listen( sfn::Endpoint{ address, 1337 } );

	// Start a network processing threads.
	sfn::Start();

	// Our source and destination sockets.
	sfn::TcpSocket::Ptr source_socket = sfn::TcpSocket::Create();
	sfn::TcpSocket::Ptr destination_socket;

	// Connect our source socket so the listener will be able to accept its connection request.
	source_socket->Connect( sfn::Endpoint{ address, 1337 } );

	// Construct our message to send.
	sfn::Message send_message;
	send_message << std::string{ "Hello World!\n" };

	// Send the message.
	source_socket->Send( send_message );

	while( true ) {
		// Check the listener for any pending connections.
		auto socket = listener->GetPendingConnection();

		// If there was a pending connection, set it as our destination socket.
		if( socket ) {
			destination_socket = socket;
		}

		// Our message to hold the received data.
		sfn::Message receive_message;

		// Dequeue the received data from our destination socket.
		while( destination_socket && destination_socket->Receive( receive_message ) ) {
			// Extract and print out the contained message.
			std::string message;
			receive_message >> message;
			std::cout << message;
		}

		// When communication on a side is done, gracefully close the connection by shutting it down.
		if( !source_socket->LocalHasShutdown() && !source_socket->BytesToSend() ) {
			source_socket->Shutdown();
		}

		if( destination_socket && destination_socket->IsConnected() && destination_socket->RemoteHasShutdown() ) {
			destination_socket->Shutdown();
			break;
		}
	}

	// Wait for the goodbye message to arrive.
	while( !source_socket->RemoteHasShutdown() ) {
	}

	// Close all sockets.
	listener->Close();
	source_socket->Close();

	if( destination_socket ) {
		destination_socket->Close();
	}

	// Stop all network processing threads.
	sfn::Stop();

	return 0;
}
