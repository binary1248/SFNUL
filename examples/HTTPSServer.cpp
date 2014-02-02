/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <iostream>
#include <atomic>
#include <SFNUL.hpp>

int main() {
	// Create our TCP listener socket.
	auto listener = sfn::TcpListener::Create();

	// Listen on 0.0.0.0:443
	listener->Listen( sfn::Endpoint{ sfn::IpAddress{ "0.0.0.0" }, 443 } );

	// Start 3 network processing threads.
	sfn::Start( 3 );

	// Just to make our lives easier...
	typedef sfn::TlsConnection<sfn::TcpSocket, sfn::TlsEndpointType::SERVER, sfn::TlsVerificationType::NONE> Connection;

	// A place to store all active connections.
	std::deque<Connection::Ptr> connections;

	const std::string certificate_string =
		"-----BEGIN CERTIFICATE-----\r\n"
		"MIIDPjCCAiagAwIBAgIBAjANBgkqhkiG9w0BAQUFADBFMQswCQYDVQQGEwJGUjEO\r\n"
		"MAwGA1UEBxMFUGFyaXMxDjAMBgNVBAoTBVh5U1NMMRYwFAYDVQQDEw1YeVNTTCBU\r\n"
		"ZXN0IENBMB4XDTA3MDcwNzA1MDEyOVoXDTA4MDcwNjA1MDEyOVowMTELMAkGA1UE\r\n"
		"BhMCRlIxDjAMBgNVBAoTBVh5U1NMMRIwEAYDVQQDEwlsb2NhbGhvc3QwggEiMA0G\r\n"
		"CSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQC40PDcGTgmHkt6noXDfkjVuymjiNYB\r\n"
		"gjtiL7uA1Ke3tXStacEecQek/OJxYqYr7ffcWalS29LL6HbKpi0xLZKBbD9ACkDh\r\n"
		"1Z/SvHlyQPILJdYb9DMw+kzZds5myXUjzn7Aem1YjoxMZUAMyc34i2900X2pL0v2\r\n"
		"SfCeJ9Ym4MOnZxYl217+dX9ZbkgIgrT6uY2IYK4boDwxbTcyT8i/NPsVsiMwtWPM\r\n"
		"rnQMr+XbgS98sUzcZE70Pe1TlV9Iy8j/8d2OiFo+qTyMu/6UpM2s3gdkQkMzx+Sm\r\n"
		"4QitRUjzmEXeUePRUjEgHIv7vz069xuVBzrks36w5BXiVAhLke/OTKVPAgMBAAGj\r\n"
		"TTBLMAkGA1UdEwQCMAAwHQYDVR0OBBYEFNkOyCTx64SDdPySGWl/tzD7/WMSMB8G\r\n"
		"A1UdIwQYMBaAFLzuH5jo+iuD5KR9XsN1cpMx2TJnMA0GCSqGSIb3DQEBBQUAA4IB\r\n"
		"AQBelJv5t+suaqy5Lo5bjNeHjNZfgg8EigDQ7NqaosvlQZAsh2N34Gg5YdkGyVdg\r\n"
		"s32I/K5aaywyUbG9qVXQxCM2T95qBqyK56h9yJoZKWQD9H//+zB8kCK/16WvRfv3\r\n"
		"VA7eSR19qOFWlHe+1qGh2YhxeDUfyi+fm4D36dGxqC2A34tZjo0QPHKtIeqM0kJy\r\n"
		"zzL65TlbJQKkyTuRHofFv0jW9ZFG2wkGysVgCY5fjuLI1do/sWUaXd2987iNFa+K\r\n"
		"FrHsTi6urSfZuGlZNxDXDHEE7Q2snAvvev+KR7DD9X4DJGcPX9gA4CGJj+9ZzyAA\r\n"
		"ZTGpOzk1hIH44RFs2lJMZRlE\r\n"
		"-----END CERTIFICATE-----\r\n";

	const std::string key_string =
		"-----BEGIN PRIVATE KEY-----\r\n"
		"MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQC40PDcGTgmHkt6\r\n"
		"noXDfkjVuymjiNYBgjtiL7uA1Ke3tXStacEecQek/OJxYqYr7ffcWalS29LL6HbK\r\n"
		"pi0xLZKBbD9ACkDh1Z/SvHlyQPILJdYb9DMw+kzZds5myXUjzn7Aem1YjoxMZUAM\r\n"
		"yc34i2900X2pL0v2SfCeJ9Ym4MOnZxYl217+dX9ZbkgIgrT6uY2IYK4boDwxbTcy\r\n"
		"T8i/NPsVsiMwtWPMrnQMr+XbgS98sUzcZE70Pe1TlV9Iy8j/8d2OiFo+qTyMu/6U\r\n"
		"pM2s3gdkQkMzx+Sm4QitRUjzmEXeUePRUjEgHIv7vz069xuVBzrks36w5BXiVAhL\r\n"
		"ke/OTKVPAgMBAAECggEAF5qHyHRoGWYxGZ35U3hjcPiQXtRxEIxDbUzPU6rVKL9C\r\n"
		"AYvKIEsrQMUTXovM0Tt5Nmk1GOH+UBCGa2dBylAZr/HVOiaVFsKjwNRVZmQpBYd1\r\n"
		"iuhrSUwOWI+12KbOER6kTYzVPkQmYvNjdL6pUZ7tQywmMl9aAkB7PJe14A1Ar7Zh\r\n"
		"K34KM8iogdyHUp7FSe903377WgTHoBu5nxPhq+EVhlA/Xm1k0ayTYt9CVhL819uj\r\n"
		"CQKx3T9NW5skGYMZQwv8TkxvGPEvOVo0HYxXn1wE2CyADfkI4MBQXjE7KhJ9kaob\r\n"
		"VFv2pCu/UTonPnp0bxKwoXceqcOPGI85UE0UGdUn4QKBgQDb0yECwoybheoFQqnq\r\n"
		"kgwVOCb+zUTm/GKDvS+oiIv6C68GGFDvOzXxQxM1dmgwQs2gH96/183QKNzVWvjp\r\n"
		"v/H/IPDvAxv3qL6/MQ07O7hEdaLc5akI9umoEoSLeHM3wz4G6U9fwINDHdotGt/H\r\n"
		"DnUnkS4DQ17JueBsi2DFOLS7qQKBgQDXOvTo/lh06Fge1JYOWR4Pj0wClxW7Noee\r\n"
		"wdlUCFSq7dHCLL0tFoktl+vt9ANv0NgltmmzFNEsbotIByxHubD6b4a82oa3/2lK\r\n"
		"pu1b4v9XBVZLIX5hiSXFlQPdQADKsNVwMQJrlL6Np3gKyL+VUe3m6VYhyRYvV8m9\r\n"
		"p2fm5so0NwKBgD7vXFUY8/6WwWBOLK1+sLzmfauXgzGKcn37DQj4RvMIo0xga9OC\r\n"
		"JTh0lqxIwR2IEqzUUwajt0XwXQEscXUiwhrkCHa1ci1ef3Xnij06JNBcyYrqqZFq\r\n"
		"d4zp+E6h5oLBgCGkbFgimrH9evhM6GJqDjqMwxqmEB46/Di3UrZEPOI5AoGBALI8\r\n"
		"IJTSDG7D+jWN1rYLFtnL0SZT96sRfT37Sf5M59ClIQ+r/P1ZrEAVj0t+x1nRmS2h\r\n"
		"4eZrVs10veLoDcNYAzdhJDNAxE+bM5aepfFyCgIGaW/OTNp4uM7mmEygtAcmaZp+\r\n"
		"+4Ibq7Gi/cXweLcvIdQXZzyTScvq5yYne+O7O7gBAoGAFIBIwAlLrcz1QkHURrPL\r\n"
		"a/Y1bDTgKbM3FR9ase4ql38LaKThAq7SxE6v0qeQODHESGMObuVeGVbXY9TjbI5G\r\n"
		"y3kjcRprUJ+Lgo8Wf+jO36I0FivN3xPK+duqR0QKXT4bYWj8RHg1EK/trbhY0Sw4\r\n"
		"wbMZGrDlydImuNtktgxojIo=\r\n"
		"-----END PRIVATE KEY-----\r\n";

	// Load the certificate and key
	auto certificate = sfn::TlsCertificate::Create( certificate_string );
	auto key = sfn::TlsKey::Create( key_string );

	std::atomic_bool exit{ false };
	std::cout << "Press ENTER to exit.\n";
	// Don't use sfn::Thread for your own projects, it is not what you think it is.
	sfn::Thread exit_handler( [&]() { std::cin.get(); exit = true; } );

	while( !exit ) {
		{
			Connection::Ptr connection;

			// Dequeue any connections from the listener.
			while( ( connection = listener->GetPendingConnection<Connection>() ) ) {
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
	}

	// Close the listener socket.
	listener->Close();

	// Stop all network processing threads.
	sfn::Stop();

	return 0;
}
