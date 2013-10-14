/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <iostream>
#include <atomic>
#include <SFNUL.hpp>

int main() {
	// Create our UDP socket.
	auto socket = sfn::UdpSocket::Create();

	// Bind the socket to 0.0.0.0:777 so we are able to echo data.
	socket->Bind( sfn::Endpoint{ sfn::IpAddress{ "0.0.0.0" }, 777 } );

	// Start a network processing thread.
	sfn::Start();

	std::atomic_bool exit{ false };
	std::cout << "Press ENTER to exit.\n";
	// Don't use sfn::Thread for your own projects, it is not what you think it is.
	sfn::Thread exit_handler( [&]() { std::cin.get(); exit = true; } );

	while( !exit ) {
		std::array<char, 1024> reply;

		// Get the endpoints with data pending in their receive queue.
		auto pending_endpoints = socket->PendingEndpoints();

		for( auto pe : pending_endpoints ) {
			// Dequeue any data the endpoint sent ...
			std::size_t reply_size = socket->ReceiveFrom( reply.data(), reply.size(), pe );

			// ... and send it right back to them.
			socket->SendTo( reply.data(), reply_size, pe );
		}
	}

	// Close the socket.
	socket->Close();

	// Stop all network processing threads.
	sfn::Stop();

	return 0;
}
