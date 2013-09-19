#include <iostream>
#include <SFML/Window.hpp>
#include <SFNUL.hpp>

int main() {
	sf::Window window{ sf::VideoMode{ 300, 100 }, "SFNUL Echo Server" };

	// Create our UDP socket.
	auto socket = sfn::UdpSocket::Create();

	// Bind the socket to 0.0.0.0:777 so we are able to echo data.
	socket->Bind( sfn::Endpoint{ sfn::IpAddress{ "0.0.0.0" }, 777 } );

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

		// Get the endpoints with data pending in their receive queue.
		auto pending_endpoints = socket->PendingEndpoints();

		for( auto pe : pending_endpoints ) {
			// Dequeue any data the endpoint sent ...
			std::size_t reply_size = socket->ReceiveFrom( reply.data(), reply.size(), pe );

			// ... and send it right back to them.
			socket->SendTo( reply.data(), reply_size, pe );
		}

		sf::sleep( sf::milliseconds( 20 ) );
	}

	// Close the socket.
	socket->Close();

	// Stop all network processing threads.
	sfn::Stop();

	return 0;
}
