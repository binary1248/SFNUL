#include <cstring>
#include <iostream>
#include <SFML/Window.hpp>
#include <SFNUL.hpp>

int main() {
	sf::Window window{ sf::VideoMode{ 300, 100 }, "SFNUL DNS Query" };

	// Create our UDP socket.
	auto socket = sfn::UdpSocket::Create();

	// Our soon-to-be DNS request.
	std::string request;

	// Transaction ID
	request += { 0x13, 0x37 };

	// Standard recursive query flags
	request += { 0x01, 0x00 };

	// Questions
	request += { 0x00, 0x01 };

	// RRs
	request += { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	// Name part 1
	request += static_cast<char>( strlen( "sfml-dev" ) );
	request += "sfml-dev";

	// Name part 2
	request += static_cast<char>( strlen( "org" ) );
	request += "org";

	// Name terminator
	request += static_cast<char>( 0x00 );

	// Type: A Class: IN
	request += { 0x00, 0x01, 0x00, 0x01 };

	// Google DNS server endpoint.
	sfn::Endpoint google_dns{ sfn::IpAddress{ "8.8.8.8" }, 53 };

	// Bind the socket to a local endpoint so we are able to receive data.
	socket->Bind( sfn::Endpoint{ sfn::IpAddress{ "0.0.0.0" }, 1337 } );

	// Send the DNS request to the Google DNS server endpoint.
	socket->SendTo( request.c_str(), request.size(), google_dns );

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

		// Dequeue any data we receive from the Google DNS server.
		std::size_t reply_size = socket->ReceiveFrom( reply.data(), reply.size(), google_dns );

		// Display the IP address so everybody believes this works.
		if( reply_size >= 4 ) {
			std::cout << "Address: " << static_cast<unsigned short>( reply[reply_size - 4] & 0xff ) << "."
			                         << static_cast<unsigned short>( reply[reply_size - 3] & 0xff ) << "."
			                         << static_cast<unsigned short>( reply[reply_size - 2] & 0xff ) << "."
			                         << static_cast<unsigned short>( reply[reply_size - 1] & 0xff ) << "\n";
		}

		sf::sleep( sf::milliseconds( 20 ) );
	}

	// Close the socket.
	socket->Close();

	// Stop all network processing threads.
	sfn::Stop();

	return 0;
}
