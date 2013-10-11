/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <iostream>
#include <SFML/Window.hpp>
#include <SFNUL.hpp>

int main( int argc, char** argv ) {
	if( argv[1] && ( argv[1][0] == 's' ) ) {
////////////////////////////////////////////////////////////////////////////////
// Server mode.
////////////////////////////////////////////////////////////////////////////////
		sf::Window window( sf::VideoMode( 300, 100 ), "SFNUL Link Example (Server)" );

		// Create our TCP listener socket.
		auto listener = sfn::TcpListener::Create();

		// Listen on 0.0.0.0:13337
		listener->Listen( sfn::Endpoint{ sfn::IpAddress{ "0.0.0.0" }, 13337 } );

		// Start a network processing thread.
		sfn::Start();

		// Our Link object using an underlying TCP transport.
		sfn::Link<sfn::TcpSocket> link;

		while( window.isOpen() ) {
			sf::Event event;

			if( window.pollEvent( event ) ) {
				if( event.type == sf::Event::Closed ) {
					window.close();
				}
			}

			// If our link isn't connected yet...
			// Same as:
			// if( !link.GetTransport()->IsConnected() ) {
			if( !link ) {
				// Try to get a pending connection.
				link.SetTransport( listener->GetPendingConnection() );
			}
			else {
				if( link.RemoteHasShutdown() ) {
					// If the remote host has requested the link to shut down,
					// shut down the our end of the link for sending as well.
					link.Shutdown();
					break;
				}

				// Send "Hello World!" on the default (0) stream.
				// If no stream identifier is provided, sending
				// and receiving defaults to stream 0.
				link.Send( "Hello World!", 13 );

				// Send "Bye World!" on the secondary (>0) stream.
				link.Send( 1, "Bye World!", 12 );
			}

			sf::sleep( sf::milliseconds( 20 ) );
		}

		// Stop all network processing threads.
		sfn::Stop();
////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
	} else {
////////////////////////////////////////////////////////////////////////////////
// Client mode.
////////////////////////////////////////////////////////////////////////////////
		sf::Window window( sf::VideoMode( 300, 100 ), "SFNUL Link Example (Client)" );

		// Resolve our hostname to an address.
		auto addresses = sfn::IpAddress::Resolve( "127.0.0.1" );

		// Check if the name resolution was unsuccessful.
		if( addresses.empty() ) {
			std::cout << "Could not resolve \"127.0.0.1\" to an address.\n";
			return 1;
		}

		// Our Link object using an underlying TCP transport.
		sfn::Link<sfn::TcpSocket> link;

		// Connect the link to the endpoint.
		// Links proxy most functions of their underlying transport object.
		// This would be equivalent to:
		// link.GetTransport()->Connect( sfn::Endpoint{ addresses.front(), 13337 } );
		link.Connect( sfn::Endpoint{ addresses.front(), 13337 } );

		// Start a network processing thread.
		sfn::Start();

		while( window.isOpen() ) {
			sf::Event event;

			if( window.pollEvent( event ) ) {
				if( event.type == sf::Event::Closed ) {
					window.close();
				}
			}

			// If our Link is connected...
			// The same as:
			// if( link.GetTransport()->IsConnected() ) {
			if( link ) {
				std::size_t received = 0;
				char buffer[1024];

				do {
					// Receive everything there is on the default (0) stream
					// and print it to cout.
					std::memset( buffer, 0, sizeof( buffer ) );
					received = link.Receive( buffer, sizeof( buffer ) );

					if( received ) {
						std::cout << "Received " << received << " bytes: " << buffer << "\n";
					}
				} while( received );

				do {
					// Receive everything there is on the secondary (>0) stream
					// and print it to cout.
					std::memset( buffer, 0, sizeof( buffer ) );
					received = link.Receive( 1, buffer, sizeof( buffer ) );

					if( received ) {
						std::cout << "Received " << received << " bytes on secondary stream: " << buffer << "\n";
					}
				} while( received );
			}

			sf::sleep( sf::milliseconds( 20 ) );
		}

		// If our Link is connected...
		if( link ) {
			// Gracefully shut down the underlying transport.
			// Same as:
			// link.GetTransport()->Shutdown();
			link.Shutdown();

			// And wait for the remote host to acknowledge the shut down.
			// Same as:
			// while( !link.GetTransport()->RemoteHasShutdown() ) {
			while( !link.RemoteHasShutdown() ) {
				sf::sleep( sf::milliseconds( 20 ) );
			}
		}

		// Stop all network processing threads.
		sfn::Stop();
////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
	}

	return 0;
}
