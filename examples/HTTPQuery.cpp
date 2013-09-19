#include <iostream>
#include <SFML/Window.hpp>
#include <SFNUL.hpp>

int main() {
	sf::Window window{ sf::VideoMode{ 300, 100 }, "SFNUL HTTP Query" };

	// Resolve our hostname to an address.
	auto addresses = sfn::IpAddress::Resolve( "sfgui.sfml-dev.de" );

	// Check if the name resolution was unsuccessful.
	if( addresses.empty() ) {
		std::cout << "Could not resolve hostname \"sfgui.sfml-dev.de\" to an address.\n";
		return 1;
	}

	// Create our TCP socket.
	auto socket = sfn::TcpSocket::Create();

	// Connect the TCP socket to the endpoint.
	socket->Connect( sfn::Endpoint{ addresses.front(), 80 } );

	// Construct our HTTP request.
	std::string request( "GET / HTTP/1.0\r\nHost: sfgui.sfml-dev.de\r\n\r\n" );

	// Send our HTTP request.
	socket->Send( request.c_str(), request.size() );

	// Start a network processing thread.
	sfn::Start();

	while( window.isOpen() ) {
		sf::Event event;

		if( window.pollEvent( event ) ) {
			if( event.type == sf::Event::Closed ) {
				window.close();
			}
		}

		std::array<char, 1024> reply;

		// Dequeue any data we receive from the remote host.
		std::size_t reply_size = socket->Receive( reply.data(), reply.size() );

		// Print out the data.
		for( std::size_t index = 0; index < reply_size; index++ ) {
			std::cout << reply[index];
		}

		// Shutdown our side if the remote side has shutdown already.
		if( socket->RemoteHasShutdown() ) {
			socket->Shutdown();
		}

		sf::sleep( sf::milliseconds( 20 ) );
	}

	// Close the socket.
	socket->Close();

	// Stop all network processing threads.
	sfn::Stop();

	return 0;
}
