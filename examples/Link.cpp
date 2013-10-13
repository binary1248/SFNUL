/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <iostream>
#include <SFNUL.hpp>

int main( int argc, char** argv ) {
	auto exit = false;
	std::cout << "Press ENTER to exit.\n";
	// Don't use sfn::Thread for your own projects, it is not what you think it is.
	sfn::Thread exit_handler( [&]() { std::cin.get(); exit = true; } );

	if( argv[1] && ( argv[1][0] == 's' ) ) {
////////////////////////////////////////////////////////////////////////////////
// Server mode.
////////////////////////////////////////////////////////////////////////////////
		// Create our TCP listener socket.
		auto listener = sfn::TcpListener::Create();

		// Listen on 0.0.0.0:13337
		listener->Listen( sfn::Endpoint{ sfn::IpAddress{ "0.0.0.0" }, 13337 } );

		// Start a network processing thread.
		sfn::Start();

		// Our Link object using an underlying TCP transport.
		sfn::Link<sfn::TcpSocket> link;

		while( !exit ) {
			// If our link isn't connected yet...
			if( !link.GetTransport() || !link.GetTransport()->IsConnected() ) {
				// Try to get a pending connection.
				link.SetTransport( listener->GetPendingConnection() );
			}
			else if( link.GetTransport()->RemoteHasShutdown() ) {
				// The remote has requested shut down, send everything that is queued.
				while( link.GetTransport()->BytesToSend() ) {
				}

				// If the remote host has requested the link to shut down,
				// shut down the our end of the link for sending as well.
				if( link.GetTransport() ) {
					link.GetTransport()->Shutdown();
					link.GetTransport()->Close();
				}

				continue;
			}
			else {
				// Send "Hello World!" on the default (0) stream.
				// If no stream identifier is provided, sending
				// and receiving defaults to stream 0.
				// If sending fails because buffers have become full,
				// Send will return false.
				if( !link.Send( "Hello World!", 13 ) ) {
					continue;
				}

				// Send "Bye World!" on the secondary (>0) stream.
				link.Send( 1, "Bye World!", 12 );
			}
		}

		// Perform graceful shut down.
		if( link.GetTransport() && link.GetTransport()->IsConnected() ) {
			while( link.GetTransport()->BytesToSend() ) {
			}

			link.GetTransport()->Shutdown();

			while( !link.GetTransport()->RemoteHasShutdown() ) {
			}

			link.GetTransport()->Close();
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
		link.GetTransport()->Connect( sfn::Endpoint{ addresses.front(), 13337 } );

		// Start a network processing thread.
		sfn::Start();

		while( !exit ) {
			// If our Link is connected...
			if( link.GetTransport()->IsConnected() ) {
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

				// If the remote has shut down, shut down our end as well.
				if( link.GetTransport()->RemoteHasShutdown() ) {
					link.GetTransport()->Shutdown();
					link.Close();
					break;
				}
			}
		}

		// If our Link is connected...
		if( link.GetTransport()->IsConnected() ) {
			// Gracefully shut down the underlying transport.
			link.GetTransport()->Shutdown();

			// And wait for the remote host to acknowledge the shut down.
			while( !link.GetTransport()->RemoteHasShutdown() ) {
				// Keep receiving until the remote has nothing left to send.
				link.GetTransport()->ClearBuffers();
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
