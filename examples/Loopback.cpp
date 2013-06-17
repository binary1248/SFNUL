#include <iostream>
#include <SFML/Window.hpp>
#include <SFNUL.hpp>

int main() {
	sf::Window window( sf::VideoMode( 300, 100 ), "SFNUL Loopback Example" );

	// Create our TCP listener socket.
	auto listener = sfn::TcpListener::Create();

	// Listen on 0.0.0.0:1337
	listener->Listen( sfn::Endpoint{ sfn::IpAddress{ "0.0.0.0" }, 1337 } );

	// Start a network processing threads.
	sfn::Start();

	sf::Clock clock;

	// Our source and destination sockets.
	sfn::TcpSocket::Ptr source_socket = sfn::TcpSocket::Create();
	sfn::TcpSocket::Ptr destination_socket;

	// Connect our source socket so the listener will be able to accept its connection request.
	source_socket->Connect( sfn::Endpoint{ sfn::IpAddress::Resolve( "localhost" ).front(), 1337 } );

	// Construct our packet to send.
	sf::Packet send_packet;
	send_packet << std::string{ "Hello World!\n" };

	// Send the packet.
	source_socket->Send( send_packet );

	while( window.isOpen() ) {
		sf::Event event;

		if( window.pollEvent( event ) ) {
			if( event.type == sf::Event::Closed ) {
				window.close();
			}
		}

		// Check the listener for any pending connections.
		auto socket = listener->GetPendingConnection();

		// If there was a pending connection, set it as our destination socket.
		if( socket ) {
			destination_socket = socket;
		}

		// Our packet to hold the received data.
		sf::Packet receive_packet;

		// Dequeue the received data from our destination socket.
		if( destination_socket && destination_socket->Receive( receive_packet ) ) {
			// Extract and print out the contained message.
			std::string message;
			receive_packet >> message;
			std::cout << message;
		}

		// When communication is done, gracefully close the connection by shutting both sides down.
		if( !source_socket->BytesToSend() ) {
			destination_socket->Shutdown();
			source_socket->Shutdown();
		}

		sf::sleep( sf::milliseconds( 20 ) - clock.restart() );
	}

	// Close all sockets.
	listener->Close();
	source_socket->Close();
	destination_socket->Close();

	// Stop all network processing threads.
	sfn::Stop();

	return 0;
}
