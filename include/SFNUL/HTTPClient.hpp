/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <SFNUL/Config.hpp>
#include <SFNUL/HTTP.hpp>
#include <SFNUL/TcpSocket.hpp>
#include <SFNUL/Endpoint.hpp>
#include <SFNUL/TlsConnection.hpp>
#include <memory>
#include <list>
#include <tuple>
#include <map>
#include <chrono>

namespace sfn {

class HTTPClientPipeline;

/** A HTTP client that supports persistent connections and request pipelining.
 */
class SFNUL_API HTTPClient {
public:
	/** Constructor.
	 */
	HTTPClient();

	/** Destructor.
	 */
	~HTTPClient();

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
	 * @param common_name Common Name to check against.
	 */
	void LoadCertificate( const std::string& address, TlsCertificate::Ptr certificate, std::string common_name = "" );

	/** Set the timeout value a connection is allowed to be idle for before being closed. Default: 15 seconds. 0 to disable.
	 * This will only have effect on new connections, so set before issuing any requests.
	 * @param timeout Timeout value in seconds.
	 */
	void SetTimeoutValue( const std::chrono::seconds& timeout );

	/** Update the client and handle any pending data/operations.
	 */
	void Update();
private:
	typedef std::tuple<std::unique_ptr<HTTPClientPipeline>, std::string, unsigned short> Pipeline;

	std::list<Pipeline> m_pipelines;
	std::map<std::string, std::pair<TlsCertificate::Ptr, std::string>> m_certificates;

	std::chrono::seconds m_timeout_value{ 15 };
};

}
