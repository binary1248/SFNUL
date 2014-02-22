/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <SFNUL/Config.hpp>

namespace sfn {

/** A HTTP message containing a header and a body.
 */
class SFNUL_API HTTPMessage {
public:
	/** Dtor.
	 */
	virtual ~HTTPMessage() = default;

	/** Get the value of a header field in this message if it is present.
	 * @param field_name Name of the header field.
	 * @return Value of the header field if it exists, or an empty std::string if not.
	 */
	std::string GetHeaderValue( const std::string& field_name ) const;

	/** Set the value of a header field in this message.
	 * If the field has not previously been set, it will be created.
	 * @param field Name of the field to set.
	 * @param value Value to set the field to.
	 */
	void SetHeaderValue( std::string field, std::string value );

	/** Get the body of this message.
	 * @return The body of this message.
	 */
	const std::string& GetBody() const;

	/** Set the body of this message.
	 * @param body The content to set the body to.
	 */
	void SetBody( std::string body );

	/** Reserve memory for a body of a specific size.
	 * size Memory to reserve in bytes.
	 */
	void ReserveBody( std::size_t size );

	/** Convert this message to its string representation according to HTTP specifications.
	 * @return String representation of this message.
	 */
	virtual std::string ToString() const;

	/** Check whether the header of this HTTP response is complete.
	 */
	bool IsHeaderComplete() const;

	/** Mark the header of this HTTP response as complete.
	 */
	void SetHeaderComplete();

	/** Check whether the body of this HTTP response is complete.
	 */
	bool IsBodyComplete() const;

	/** Mark the body of this HTTP response as complete.
	 */
	void SetBodyComplete();

protected:
	friend bool operator==( const HTTPMessage& left, const HTTPMessage& right );

	std::unordered_map<std::string, std::string> m_header;
	std::string m_body;

	bool m_header_complete{ false };
	bool m_body_complete{ false };
};

#if defined( __GNUG__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#endif

/** A HTTP request containing a method, uri and remaining HTTP message.
 */
class SFNUL_API HTTPRequest : public HTTPMessage {
public:
	/** Ctor.
	 */
	HTTPRequest();

	/** Get the method this HTTP request should employ.
	 * @return The method this HTTP request should employ.
	 */
	const std::string& GetMethod() const;

	/** Set the method this HTTP request should employ.
	 * @param method The method this HTTP request should employ.
	 */
	void SetMethod( std::string method );

	/** Get the URI of this HTTP request.
	 * @return The URI of this HTTP request.
	 */
	const std::string& GetURI() const;

	/** Set the URI of this HTTP request.
	 * @param uri The URI of this HTTP request.
	 */
	void SetURI( std::string uri );

	/** Convert this request to its string representation according to HTTP specifications.
	 * @return String representation of this request.
	 */
	std::string ToString() const override;

protected:
	std::string m_method;
	std::string m_uri;
};

/** A HTTP response containing a version, status and remaining HTTP message.
 */
class SFNUL_API HTTPResponse : public HTTPMessage {
public:
	/** Get the HTTP version of this response. e.g. "HTTP/1.1"
	 * @return HTTP version of this response.
	 */
	const std::string& GetHTTPVersion() const;

	/** Set the HTTP version of this response. e.g. "HTTP/1.1"
	 * @param version The HTTP version of this response.
	 */
	void SetHTTPVersion( std::string version );

	/** Get the HTTP status of this response in string form. e.g. "200 OK"
	 * @return The HTTP status of this response in string form.
	 */
	const std::string& GetStatus() const;

	/** Set the HTTP status of this response in string form. e.g. "200 OK"
	 * @param status The HTTP status of this response in string form.
	 */
	void SetStatus( std::string status );

	/** Convert this response to its string representation according to HTTP specifications.
	 * @return String representation of this response.
	 */
	std::string ToString() const override;

	/** Check if this HTTP response has arrived and been parsed completely.
	 * @return true if this HTTP response has arrived and been parsed completely.
	 */
	bool IsComplete() const;

	/** Get the header fields present in this HTTP response.
	 * @return Header fields present in this HTTP response.
	 */
	std::vector<std::string> GetHeaderFields() const;

protected:
	std::string m_http_version;
	std::string m_status;
};

#if defined( __GNUG__ )
#pragma GCC diagnostic pop
#endif

bool operator==( const HTTPMessage& left, const HTTPMessage& right );

bool operator==( const HTTPRequest& left, const HTTPRequest& right );

bool operator==( const HTTPResponse& left, const HTTPResponse& right );

}
