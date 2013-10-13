/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <list>
#include <deque>
#include <tuple>
#include <map>
#include <chrono>
#include <http_parser.h>
#include <SFNUL/Config.hpp>
#include <SFNUL/HTTP.hpp>
#include <SFNUL/TcpSocket.hpp>
#include <SFNUL/Endpoint.hpp>
#include <SFNUL/TlsConnection.hpp>

namespace sfn {

/// @cond

class SFNUL_API HTTPClientPipeline {
public:
	HTTPClientPipeline( Endpoint endpoint, bool secure, const std::chrono::seconds& timeout );

	HTTPClientPipeline( HTTPClientPipeline&& ) = default;

	~HTTPClientPipeline();

	void LoadCertificate( TlsCertificate::Ptr certificate );

	void SendRequest( HTTPRequest request );

	HTTPResponse GetResponse( const HTTPRequest& request );

	void Update();

	bool TimedOut() const;

	bool HasRequests() const;
private:
	friend int OnMessageBegin( http_parser* parser );
	friend int OnUrl( http_parser* parser, const char* data, std::size_t length );
	friend int OnStatusComplete( http_parser* parser );
	friend int OnHeaderField( http_parser* parser, const char* data, std::size_t length );
	friend int OnHeaderValue( http_parser* parser, const char* data, std::size_t length );
	friend int OnHeadersComplete( http_parser* parser );
	friend int OnBody( http_parser* parser, const char* data, std::size_t length );
	friend int OnMessageComplete( http_parser* parser );

	typedef std::pair<HTTPRequest, HTTPResponse> PipelineElement;
	typedef std::deque<PipelineElement> Pipeline;

	void Reconnect();

	TcpSocket::Ptr m_socket{};

	bool m_secure;
	Endpoint m_remote_endpoint;
	TlsCertificate::Ptr m_certificate{};

#if defined( __GNUG__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

	http_parser_settings m_parser_settings{};
	http_parser m_parser{};

#if defined( __GNUG__ )
#pragma GCC diagnostic pop
#endif

	Pipeline m_pipeline{};

	HTTPRequest m_current_request{};

	std::string m_last_header_field{};
	bool m_header_field_complete{ false };

	std::chrono::steady_clock::time_point m_last_activity{ std::chrono::steady_clock::now() };

	std::chrono::seconds m_timeout_value;
};

/// @endcond

/** A HTTP client that supports persistent connections and request pipelining.
 */
class SFNUL_API HTTPClient {
public:
	/** Send an HTTP request to a host identified by address, port and whether it is secured by TLS or not (HTTPS).
	 * @param request Request to send to the host.
	 * @param address Address of the host.
	 * @param port Port of the host. Default: 80
	 * @param secure true to establish a TLS secured connection. Default: false
	 */
	void SendRequest( HTTPRequest request, const std::string& address, unsigned short port = 80, bool secure = false );

	/** Get a response to a previously send request from a connected host identified by address and port.
	 * @param request Associated request.
	 * @param address Address of the connected host.
	 * @param port Port of the connected host. Default: 80
	 * @return HTTP Response associated with the specified request. It might be incomplete, so check before use.
	 */
	HTTPResponse GetResponse( const HTTPRequest& request, const std::string& address, unsigned short port = 80 );

	/** Load a certificate to use for a certain host.
	 * @param address Address of the host.
	 * @param certificate Certificate to use.
	 */
	void LoadCertificate( const std::string& address, TlsCertificate::Ptr certificate );

	/** Set the timeout value a connection is allowed to be idle for before being closed. Default: 15 seconds. 0 to disable.
	 * This will only have effect on new connections, so set before issuing any requests.
	 * @param timeout Timeout value in seconds.
	 */
	void SetTimeoutValue( const std::chrono::seconds& timeout );

	/** Update the client and handle any pending data/operations.
	 */
	void Update();
private:
	typedef std::tuple<HTTPClientPipeline, std::string, unsigned short> Pipeline;

	std::list<Pipeline> m_pipelines{};
	std::map<std::string, TlsCertificate::Ptr> m_certificates{};

	std::chrono::seconds m_timeout_value{ 15 };
};

}
