/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <SFNUL/Config.hpp>
#include <memory>
#include <vector>
#include <type_traits>
#include <string>
#include <array>
#include <cstdint>
#include <functional>

namespace sfn {

class ReliableTransport;
class Endpoint;
class Message;

enum class TlsEndpointType : unsigned char {
	Client = 0,
	Server
};

enum class TlsVerificationType : unsigned char {
	None = 0,
	Optional,
	Required
};

enum class TlsVerificationResult : unsigned char {
	Passed = 1 << 0,
	Expired = 1 << 1,
	Revoked = 1 << 2,
	CnMismatch = 1 << 3,
	NotTrusted = 1 << 4
};

#if defined( SFNUL_BROKEN_CXX11 )
#define CONSTEXPR
#else
#define CONSTEXPR constexpr
#endif

inline CONSTEXPR TlsVerificationResult operator&( TlsVerificationResult left, TlsVerificationResult right ) {
	return static_cast<TlsVerificationResult>( static_cast<unsigned char>( left ) & static_cast<unsigned char>( right ) );
}

inline CONSTEXPR TlsVerificationResult operator|( TlsVerificationResult left, TlsVerificationResult right ) {
	return static_cast<TlsVerificationResult>( static_cast<unsigned char>( left ) | static_cast<unsigned char>( right ) );
}

inline TlsVerificationResult& operator|=( TlsVerificationResult& left, TlsVerificationResult right ) {
	return left = left | right;
}

class SFNUL_API TlsResource {

protected:
	TlsResource();
	~TlsResource() = default;
private:
	class LibraryImpl;
	std::shared_ptr<LibraryImpl> m_library;

	friend class TlsKey;
	friend class TlsConnectionBase;
};

template<class T, TlsEndpointType U, TlsVerificationType V> class TlsConnection;

/** TLS Certificate class.
 */
class SFNUL_API TlsCertificate : protected TlsResource {

public:
	typedef std::shared_ptr<TlsCertificate> Ptr; //!< Shared pointer.

	/** Dtor.
	 */
	~TlsCertificate();

	/** Create a TlsCertificate from a certificate string.
	 * @param certificate std::string containing the Base 64 DER encoded certificate.
	 * @return TlsCertificate
	 */
	static Ptr Create( const std::string& certificate );

protected:
	/** Constructor.
	 */
	TlsCertificate();

private:
	void LoadCertificate( const std::string& certificate );

	class TlsCertificateImpl;
	std::unique_ptr<TlsCertificateImpl> m_impl;

	friend class TlsConnectionBase;
};

/** TLS Key class.
 */
class SFNUL_API TlsKey : protected TlsResource {

public:
	typedef std::shared_ptr<TlsKey> Ptr; //!< Shared pointer.

	/** Dtor.
	 */
	~TlsKey();

	/** Create a TlsKey from a key string and optional password.
	 * @param key std::string containing the PKCS8 key.
	 * @param password std::string containing the optional password.
	 * @return TlsKey
	 */
	static Ptr Create( const std::string& key, const std::string& password = std::string{} );

protected:
	/** Constructor.
	 */
	TlsKey();

private:
	void LoadKey( const std::string& key, const std::string& password );

	class TlsKeyImpl;
	std::unique_ptr<TlsKeyImpl> m_impl;

	friend class TlsConnectionBase;
};

/** TLS connection base.
 */
class SFNUL_API TlsConnectionBase : protected TlsResource {

public:
	/** Add a trusted CA certificate to the certificate store to verify the peer against.
	 * The certificate must be encoded in Base64 DER format.
	 * @param certificate TlsCertificate containing the X.509 certificate to trust.
	 */
	void AddTrustedCertificate( TlsCertificate::Ptr certificate );

	/** Set the Common Name to verify the peer certificate against.
	 * @param name Expected Common Name.
	 */
	void SetPeerCommonName( const std::string& name );

	/** Set the server certificate and keys.
	 * The certificate and key must be encoded in Base64 DER format.
	 * @param certificate TlsCertificate containing the X.509 server certificate.
	 * @param key TlsKey containing the X.509 key.
	 */
	void SetCertificateKeyPair( TlsCertificate::Ptr certificate, TlsKey::Ptr key );

	/** Get the result of the certificate verification.
	 * @return Bitwise ORed verification flags.
	 */
	TlsVerificationResult GetVerificationResult() const;

protected:
	TlsConnectionBase( TlsEndpointType type, TlsVerificationType verify );
	virtual ~TlsConnectionBase();

	/// @cond
	typedef const void* AlertType;
	typedef const void* SessionType;

	typedef std::function<void (const std::uint8_t*, std::size_t)> OutputFunction;
	typedef std::function<void (const std::uint8_t*, std::size_t)> DataFunction;
	typedef std::function<void (AlertType, const std::uint8_t*, std::size_t)> AlertFunction;
	typedef std::function<bool (SessionType)> HandshakeFunction;

	void SetEndpoint( TlsEndpointType type, OutputFunction output_callback, DataFunction data_callback, AlertFunction alert_callback, HandshakeFunction handshake_callback );
	bool EndpointIsActive() const;
	void CloseEndpoint();
	void ResetEndpoint();
	void EndpointSend( const unsigned char* data, std::size_t size );
	void EndpointReceive( const unsigned char* data, std::size_t size );
	/// @endcond

	TlsEndpointType m_type;
	TlsVerificationType m_verify;

private:
	class TlsConnectionBaseImpl;
	std::unique_ptr<TlsConnectionBaseImpl> m_impl;
};

/** TLS connection class.
 */
template<class T, TlsEndpointType U, TlsVerificationType V>
class TlsConnection : public T, public TlsConnectionBase {

public:
	static_assert( std::is_base_of<ReliableTransport, T>::value, "TLS connections can only be set up over reliable transports." );

	typedef std::shared_ptr<TlsConnection<T, U, V>> Ptr; //!< Shared pointer.

	/** Create a TLS connection.
	 * @return A shared pointer that owns a new TLS connection.
	 */
	static Ptr Create();

	/** Destructor.
	 */
	virtual ~TlsConnection();

	/** Asynchronously connect to a remote endpoint.
	 * @param endpoint Remote endpoint.
	 */
	void Connect( const Endpoint& endpoint ) override;

	/** Shutdown the connection for sending. This is required for graceful connection termination.
	 */
	void Shutdown() override;

	/** Check if the local system has shut the connection down for sending.
	 * @return true if the local system has shut the connection down for sending.
	 */
	bool LocalHasShutdown() const override;

	/** Check if the remote system has shut the connection down for sending.
	 * @return true if the remote system has shut the connection down for sending.
	 */
	bool RemoteHasShutdown() const override;

	/** Check if the TLS connection is operational.
	 * @return true if the TLS connection is operational.
	 */
	bool IsConnected() const override;

	/** Close the connection. This frees up the operating system resources assigned to the connection.
	 */
	void Close() override;

	/// @cond
	void Reset() override;
	/// @endcond

	/** Queue data up for asynchronous sending over the connection.
	 * @param data Pointer to a block of memory containing the data to queue.
	 * @param size Size of the block of memory containing the data to queue.
	 * @return true if the data could be queued. If false is returned, retry again later.
	 */
	bool Send( const void* data, std::size_t size ) override;

	/** Dequeue data that was asynchronously received over the connection.
	 * @param data Pointer to a block of memory that will contain the data to dequeue.
	 * @param size Size of the block of memory that will contain the data to dequeue.
	 * @return Number of bytes of data that was actually dequeued.
	 */
	std::size_t Receive( void* data, std::size_t size ) override;

	/** Queue a Message up for asynchronous sending over the connection.
	 * @param message Message to queue.
	 * @return true if the message could be queued. If false is returned, retry again later.
	 */
	bool Send( const Message& message ) override;

	/** Dequeue an Message that was asynchronously received over the connection.
	 * @param message Message to dequeue into.
	 * @return Size of the Message that was dequeued. This includes the size field of the Message. If no Message could be dequeued, this method will return 0.
	 */
	std::size_t Receive( Message& message ) override;

	/** Clear the send and receive queues of this socket.
	 */
	void ClearBuffers() override;

	/** Get the number of bytes queued for sending.
	 * @return Number of bytes queued for sending.
	 */
	std::size_t BytesToSend() const override;

	/** Get the number of bytes queued for receiving.
	 * @return Number of bytes queued for receiving.
	 */
	std::size_t BytesToReceive() const override;

	/// @cond
	void OnConnected() override;
	/// @endcond

protected:
	/** Constructor.
	 */
	TlsConnection();

private:
	void OnReceived() override;
	void OnDisconnected() override;

	void OutputCallback( const unsigned char* buffer, std::size_t size );
	void DataCallback( const unsigned char* buffer, std::size_t size );
	void AlertCallback( AlertType, const unsigned char*, size_t );
	bool HandshakeCallback( SessionType );

	std::vector<char> m_send_buffer;
	std::vector<char> m_receive_buffer;

	std::array<char, 2048> m_send_memory;
	std::array<char, 2048> m_receive_memory;

	bool m_request_close = false;
	bool m_remote_closed = false;
	bool m_local_closed = false;
};

}

#include <SFNUL/TlsConnection.inl>
