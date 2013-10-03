#include <iostream>
#include <SFML/Window.hpp>
#include <SFNUL.hpp>

int main() {
	sf::Window window{ sf::VideoMode{ 300, 100 }, "SFNUL HTTP Query" };

	// Start a network processing thread.
	sfn::Start();

	// We enclose the HTTPClient in a sub-scope so that it can get destroyed
	// before sfn::Stop() is called and thereby clean up its connections properly.
	{
		// Construct our HTTP request.
		sfn::HTTPRequest request{};
		request.SetMethod( "GET" );
		request.SetHeaderValue( "Host", "sfgui.sfml-dev.de" );

		// Construct our HTTP client.
		sfn::HTTPClient client;

		// Our sender lambda for convenience.
		auto send_request = [&] ( std::string uri ) {
			request.SetURI( uri );
			client.SendRequest( request, "sfgui.sfml-dev.de", 80, false );
		};

		// Send a few pipelined requests.
		send_request( "/" );
		send_request( "/download/" );
		send_request( "/p/docs" );
		send_request( "/p/contributing" );
		send_request( "/p/contact" );

		while( window.isOpen() ) {
			sf::Event event;

			if( window.pollEvent( event ) ) {
				if( event.type == sf::Event::Closed ) {
					window.close();
				}
			}

			// Update our client.
			client.Update();

			// Our getter lambda for convenience.
			auto get_response = [&]( std::string uri ) {
				// Check if the responses to our requests have arrived.
				request.SetURI( uri );
				auto response = client.GetResponse( request, "sfgui.sfml-dev.de", 80 );

				// Check if the response is complete and valid. Do not use it otherwise.
				if( response.IsComplete() ) {
					// Print the body of the response.
					std::cout << response.GetBody();
				}
			};

			// Get responses with the help of our lambda.
			get_response( "/" );
			get_response( "/download/" );
			get_response( "/p/docs" );
			get_response( "/p/contributing" );
			get_response( "/p/contact" );

			sf::sleep( sf::milliseconds( 20 ) );
		}
	}

	// Stop all network processing threads.
	sfn::Stop();

	return 0;
}
