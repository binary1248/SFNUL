/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <sstream>
#include <SFNUL/HTTP.hpp>

namespace sfn {

std::string HTTPMessage::GetHeaderValue( const std::string& field_name ) const {
	auto iter = m_header.find( field_name );

	if( iter != std::end( m_header ) ) {
		return iter->second;
	}

	return "";
}

void HTTPMessage::SetHeaderValue( std::string field, std::string value ) {
	m_header[field] = std::move( value );
}

std::string HTTPMessage::GetBody() const {
	return m_body;
}

void HTTPMessage::SetBody( std::string body ) {
	m_body = std::move( body );
}

std::string HTTPMessage::ToString() const {
	std::string str;

	for( auto h : m_header ) {
		str += h.first + ": " + h.second + "\r\n";
	}

	str += "\r\n";

	str += m_body;

	return str;
}

bool operator==( const HTTPMessage& left, const HTTPMessage& right ) {
	return ( left.m_header == right.m_header ) && ( left.GetBody() == right.GetBody() );
}

std::string HTTPRequest::GetMethod() const {
	return m_method;
}

void HTTPRequest::SetMethod( std::string method ) {
	m_method = std::move( method );
}

std::string HTTPRequest::GetURI() const {
	return m_uri;
}

void HTTPRequest::SetURI( std::string uri ) {
	m_uri = std::move( uri );
}

std::string HTTPRequest::ToString() const {
	return m_method + " " + m_uri + " HTTP/1.1\r\n" + HTTPMessage::ToString();
}

bool operator==( const HTTPRequest& left, const HTTPRequest& right ) {
	return ( left.GetMethod() == right.GetMethod() ) &&
	       ( left.GetURI() == right.GetURI() ) &&
	       ( static_cast<HTTPMessage>( left ) == static_cast<HTTPMessage>( right ) );
}

std::string HTTPResponse::GetHTTPVersion() const {
	return m_http_version;
}

void HTTPResponse::SetHTTPVersion( std::string version ) {
	m_http_version = std::move( version );
}

std::string HTTPResponse::GetStatus() const {
	return m_status;
}

void HTTPResponse::SetStatus( std::string status ) {
	m_status = std::move( status );
}

std::string HTTPResponse::ToString() const {
	return m_http_version + " " + m_status + "\r\n" + HTTPMessage::ToString();
}

bool HTTPResponse::IsComplete() const {
	return m_complete;
}

void HTTPResponse::SetComplete() {
	m_complete = true;
}

bool operator==( const HTTPResponse& left, const HTTPResponse& right ) {
	return ( left.GetHTTPVersion() == right.GetHTTPVersion() ) &&
	       ( left.GetStatus() == right.GetStatus() ) &&
	       ( static_cast<HTTPMessage>( left ) == static_cast<HTTPMessage>( right ) );
}

}
