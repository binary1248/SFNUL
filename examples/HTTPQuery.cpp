/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <iostream>
#include <SFNUL.hpp>

int main() {
	// Start a network processing thread.
	sfn::Start();

	// We enclose the HTTPClient in a sub-scope so that it can get destroyed
	// before sfn::Stop() is called and thereby clean up its connections properly.
	{
		// Construct our HTTP request.
		sfn::HTTPRequest request{};
		request.SetMethod( "GET" );
		request.SetHeaderValue( "Host", "www.ietf.org" );

		// Construct our HTTP client.
		sfn::HTTPClient client;

		// Our sender lambda for convenience.
		auto send_request = [&] ( std::string uri ) {
			request.SetURI( uri );
			client.SendRequest( request, "www.ietf.org", 80, false );
		};

		// Send a few pipelined requests.
		send_request( "/" );
		send_request( "/rfc/rfc768.txt" );
		send_request( "/rfc/rfc791.txt" );
		send_request( "/rfc/rfc793.txt" );

		int complete = 0;

		// Our getter lambda for convenience.
		auto get_response = [&]( std::string uri ) {
			// Check if the responses to our requests have arrived.
			request.SetURI( uri );
			auto response = client.GetResponse( request, "www.ietf.org", 80 );

			// Check if the header is complete and valid. Do not use it otherwise.
			if( response.IsHeaderComplete() ) {
				// Get the value in the "Content-Length" field.
				auto content_length_str = response.GetHeaderValue( "Content-Length" );

				// Check if the "Content-Length" field exists.
				if( !content_length_str.empty() ) {
					// Convert the string length to an integer.
					auto content_length = std::stoi( content_length_str );

					if( content_length ) {
						// Print out the transfer status for the given URI.
						std::cout << uri << ": Received " << response.GetBody().length()
						          << " out of " << content_length << " bytes.\n";
					}
				}
			}

			// Check if the body is complete and valid. Do not use it otherwise.
			if( response.IsBodyComplete() ) {
				// Print the body of the response.
				std::cout << response.GetBody();
				++complete;
			}
		};

		while( complete < 4 ) {
			// Update our client.
			client.Update();

			// Get responses with the help of our lambda.
			get_response( "/" );
			get_response( "/rfc/rfc768.txt" );
			get_response( "/rfc/rfc791.txt" );
			get_response( "/rfc/rfc793.txt" );
		}
	}

	// Stop all network processing threads.
	sfn::Stop();

	return 0;
}
