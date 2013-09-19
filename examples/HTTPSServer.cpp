#include <iostream>
#include <SFML/Window.hpp>
#include <SFNUL.hpp>

int main() {
	sf::Window window{ sf::VideoMode{ 300, 100 }, "SFNUL TLS Echo Server" };

	// Create our TCP listener socket.
	auto listener = sfn::TcpListener::Create();

	// Listen on 0.0.0.0:443
	listener->Listen( sfn::Endpoint{ sfn::IpAddress{ "0.0.0.0" }, 443 } );

	// Start 3 network processing threads.
	sfn::Start( 3 );

	sf::Clock clock;

	// Just to make our lives easier...
	typedef sfn::TlsConnection<sfn::TcpSocket, sfn::TlsEndpointType::SERVER, sfn::TlsVerificationType::NONE> Connection;

	// A place to store all active connections.
	std::deque<Connection::Ptr> connections;

	// Load the certificate and key from the stock ones provided by TropicSSL.
	auto certificate = sfn::TlsCertificate::Create( test_srv_crt );
	auto key = sfn::TlsKey::Create( test_srv_key );

	while( window.isOpen() ) {
		sf::Event event;

		if( window.pollEvent( event ) ) {
			if( event.type == sf::Event::Closed ) {
				window.close();
			}
		}

		{
			Connection::Ptr connection;

			// Dequeue any connections from the listener.
			while( connection = listener->GetPendingConnection<Connection>() ) {
				// Set the server certificate and key pair.
				connection->SetCertificateKeyPair( certificate, key );

				// Turn of connection lingering.
				connection->SetLinger( 0 );

				char response[] =
					"HTTP/1.1 200 OK\r\n"
					"Server: SFNUL HTTPS Server\r\n"
					"Content-Type: text/html; charset=UTF-8\r\n"
					"Connection: close\r\n\r\n"
					"<html><head><title>SFNUL HTTPS Server Page</title></head>"
					"<body>SFNUL HTTPS Server Document</body></html>\r\n\r\n";

				// Send the HTTP response.
				connection->Send( response, sizeof( response ) - 1 );

				// Shutdown the connection for sending.
				connection->Shutdown();

				// Store the connection.
				connections.push_back( connection );
			}
		}

		for( auto connection_iter = connections.begin(); connection_iter != connections.end(); ) {
			// Remove and thereby close all active connections that have been
			// remotely shut down and don't have data left to send.
			if( ( *connection_iter )->RemoteHasShutdown() && !( *connection_iter )->BytesToSend() ) {
				//( *connection_iter )->Shutdown();
				connection_iter = connections.erase( connection_iter );
			}
			else {
				++connection_iter;
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
