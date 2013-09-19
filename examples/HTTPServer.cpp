#include <iostream>
#include <SFML/Window.hpp>
#include <SFNUL.hpp>

int main() {
	sf::Window window{ sf::VideoMode{ 300, 100 }, "SFNUL HTTP Server" };

	// Create our TCP listener socket.
	auto listener = sfn::TcpListener::Create();

	// Listen on 0.0.0.0:80
	listener->Listen( sfn::Endpoint{ sfn::IpAddress{ "0.0.0.0" }, 80 } );

	// Start 3 network processing threads.
	sfn::Start( 3 );

	sf::Clock clock;

	// A place to store all active connections.
	std::deque<sfn::TcpSocket::Ptr> sockets;

	while( window.isOpen() ) {
		sf::Event event;

		if( window.pollEvent( event ) ) {
			if( event.type == sf::Event::Closed ) {
				window.close();
			}
		}

		sfn::TcpSocket::Ptr socket;

		// Dequeue any pending connections from the listener.
		while( socket = listener->GetPendingConnection() ) {
			char response[] =
				"HTTP/1.1 200 OK\r\n"
				"Server: SFNUL HTTP Server\r\n"
				"Content-Type: text/html; charset=UTF-8\r\n"
				"Connection: close\r\n\r\n"
				"<html><head><title>SFNUL HTTP Server Page</title></head>"
				"<body>SFNUL HTTP Server Document</body></html>\r\n\r\n";

			// Turn of connection lingering.
			socket->SetLinger( 0 );

			// Send the HTTP response.
			socket->Send( response, sizeof( response ) - 1 );

			// Shutdown the socket for sending.
			socket->Shutdown();

			// Store the socket.
			sockets.push_back( socket );
		}

		for( auto socket_iter = sockets.begin(); socket_iter != sockets.end(); ) {
			// Remove and thereby close all active sockets that have been
			// remotely shut down and don't have data left to send.
			if( ( *socket_iter )->RemoteHasShutdown() && !( *socket_iter )->BytesToSend() ) {
				socket_iter = sockets.erase( socket_iter );
			}
			else {
				++socket_iter;
			}
		}

		sf::sleep( sf::milliseconds( 20 ) - clock.restart() );
	}

	// Close the listener socket.
	listener->Close();

	// Stop all network processing threads.
	sfn::Stop();

	return 0;
}
