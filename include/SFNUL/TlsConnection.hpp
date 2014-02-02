/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <memory>
#include <vector>
#include <type_traits>
#include <string>
#include <array>
#include <botan/tls_client.h>
#include <botan/tls_server.h>
#include <botan/pkcs8.h>
#include <botan/auto_rng.h>
#include <botan/init.h>
#include <SFNUL/Config.hpp>

#if defined( NONE )
#undef NONE
#endif

#if defined( OPTIONAL )
#undef OPTIONAL
#endif

#if defined( REQUIRED )
#undef REQUIRED
#endif

namespace sfn {

class ReliableTransport;
class Endpoint;
class Message;

enum class TlsEndpointType : unsigned char {
	CLIENT = 0,
	SERVER
};

enum class TlsVerificationType : unsigned char {
	NONE = 0,
	OPTIONAL,
	REQUIRED
};

enum class TlsVerificationResult : unsigned char {
	PASSED = 0,
	EXPIRED = 1 << 0,
	REVOKED = 1 << 1,
	CN_MISMATCH = 1 << 2,
	NOT_TRUSTED = 1 << 3
};

inline constexpr TlsVerificationResult operator&( TlsVerificationResult left, TlsVerificationResult right ) {
	return static_cast<TlsVerificationResult>( static_cast<unsigned char>( left ) & static_cast<unsigned char>( right ) );
}

inline constexpr TlsVerificationResult operator|( TlsVerificationResult left, TlsVerificationResult right ) {
	return static_cast<TlsVerificationResult>( static_cast<unsigned char>( left ) | static_cast<unsigned char>( right ) );
}

inline TlsVerificationResult& operator|=( TlsVerificationResult& left, TlsVerificationResult right ) {
	return left = left | right;
}

class SFNUL_API BotanResource {

protected:
	BotanResource();
	~BotanResource() = default;
private:
	static std::weak_ptr<Botan::LibraryInitializer> m_library_initializer;

	std::shared_ptr<Botan::LibraryInitializer> m_library_initializer_reference{};
};

template<class T, TlsEndpointType U, TlsVerificationType V> class TlsConnection;

#if defined( __GNUG__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#endif

/** TLS Certificate class.
 */
class SFNUL_API TlsCertificate : protected BotanResource {

#if defined( __GNUG__ )
#pragma GCC diagnostic pop
#endif

public:
	typedef std::shared_ptr<TlsCertificate> Ptr; //!< Shared pointer.

	/** Dtor.
	 */
	~TlsCertificate() = default;

	/** Create a TlsCertificate from a certificate string.
	 * @param certificate std::string containing the Base 64 DER encoded certificate.
	 * @return TlsCertificate
	 */
	static Ptr Create( const std::string& certificate );

protected:
	/** Ctor.
	 */
	TlsCertificate() = default;

private:
	void LoadCertificate( const std::string& certificate );

	std::unique_ptr<Botan::X509_Certificate> m_certificate{};

	friend class TlsConnectionBase;
};

#if defined( __GNUG__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#endif

/** TLS Key class.
 */
class SFNUL_API TlsKey : protected BotanResource {

#if defined( __GNUG__ )
#pragma GCC diagnostic pop
#endif

public:
	typedef std::shared_ptr<TlsKey> Ptr; //!< Shared pointer.

	/** Dtor.
	 */
	~TlsKey() = default;

	/** Create a TlsKey from a key string and optional password.
	 * @param key std::string containing the PKCS8 key.
	 * @param password std::string containing the optional password.
	 * @return TlsKey
	 */
	static Ptr Create( const std::string& key, const std::string& password = std::string{} );

protected:
	/** Ctor.
	 */
	TlsKey() = default;

private:
	void LoadKey( const std::string& key, const std::string& password );

	Botan::AutoSeeded_RNG m_rng{};
	Botan::Private_Key* m_key{};

	friend class TlsConnectionBase;
};

#if defined( __GNUG__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#endif

/** TLS connection base.
 */
class SFNUL_API TlsConnectionBase : public Botan::Credentials_Manager, protected BotanResource {

#if defined( __GNUG__ )
#pragma GCC diagnostic pop
#endif

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
	TlsConnectionBase() = default;
	virtual ~TlsConnectionBase() = default;

	std::vector<Botan::Certificate_Store*> trusted_certificate_authorities( const std::string& type, const std::string& /*context*/ ) override;
	void verify_certificate_chain( const std::string& type, const std::string& hostname, const std::vector<Botan::X509_Certificate>& certificate_chain ) override;

	std::vector<Botan::X509_Certificate> cert_chain( const std::vector<std::string>& cert_key_types, const std::string& /*type*/, const std::string& /*context*/ ) override;
	Botan::Private_Key* private_key_for( const Botan::X509_Certificate& /*cert*/, const std::string& /*type*/, const std::string& /*context*/ ) override;

	Botan::SymmetricKey psk( const std::string& /*type*/, const std::string& /*context*/, const std::string& identity ) override;
	std::string psk_identity( const std::string& /*type*/, const std::string& /*context*/, const std::string& /*identity_hint*/ ) override;
	std::string psk_identity_hint( const std::string& /*type*/, const std::string& /*context*/ ) override;

	bool attempt_srp( const std::string& /*type*/, const std::string& /*context*/ ) override;
	std::string srp_identifier( const std::string& /*type*/, const std::string& /*context*/ ) override;
	std::string srp_password( const std::string& /*type*/, const std::string& /*context*/, const std::string& /*identifier*/ ) override;
	bool srp_verifier( const std::string& /*type*/, const std::string& context, const std::string& /*identifier*/, std::string& /*group_name*/, Botan::BigInt& /*verifier*/, std::vector<unsigned char>& /*salt*/, bool /*generate_fake_on_unknown*/ ) override;

	Botan::AutoSeeded_RNG m_rng{};
	Botan::TLS::Policy m_tls_policy{};
	Botan::TLS::Session_Manager_In_Memory m_tls_session_manager{ m_rng };

	Botan::SymmetricKey m_session_ticket_key{};

	TlsVerificationResult m_last_verification_result{ TlsVerificationResult::NOT_TRUSTED };

	bool require_certificate_key = false;

private:
	std::vector<std::unique_ptr<Botan::Certificate_Store>> m_certificate_stores{};

	TlsCertificate::Ptr m_server_cert{};
	TlsKey::Ptr m_key{};

	std::string m_common_name{};
};

#if defined( __GNUG__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#endif

/** TLS connection class.
 */
template<class T, TlsEndpointType U, TlsVerificationType V>
class TlsConnection : public T, public TlsConnectionBase {

#if defined( __GNUG__ )
#pragma GCC diagnostic pop
#endif

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
	virtual void Connect( const Endpoint& endpoint ) override;

	/** Shutdown the connection for sending. This is required for graceful connection termination.
	 */
	virtual void Shutdown() override;

	/** Check if the local system has shut the connection down for sending.
	 * @return true if the local system has shut the connection down for sending.
	 */
	virtual bool LocalHasShutdown() const override;

	/** Check if the remote system has shut the connection down for sending.
	 * @return true if the remote system has shut the connection down for sending.
	 */
	virtual bool RemoteHasShutdown() const override;

	/** Check if the TLS connection is operational.
	 * @return true if the TLS connection is operational.
	 */
	virtual bool IsConnected() const override;

	/** Close the connection. This frees up the operating system resources assigned to the connection.
	 */
	virtual void Close() override;

	/// @cond
	virtual void Reset() override;
	/// @endcond

	/** Queue data up for asynchronous sending over the connection.
	 * @param data Pointer to a block of memory containing the data to queue.
	 * @param size Size of the block of memory containing the data to queue.
	 * @return true if the data could be queued. If false is returned, retry again later.
	 */
	virtual bool Send( const void* data, std::size_t size ) override;

	/** Dequeue data that was asynchronously received over the connection.
	 * @param data Pointer to a block of memory that will contain the data to dequeue.
	 * @param size Size of the block of memory that will contain the data to dequeue.
	 * @return Number of bytes of data that was actually dequeued.
	 */
	virtual std::size_t Receive( void* data, std::size_t size ) override;

	/** Queue a Message up for asynchronous sending over the connection.
	 * @param message Message to queue.
	 * @return true if the message could be queued. If false is returned, retry again later.
	 */
	virtual bool Send( const Message& message ) override;

	/** Dequeue an Message that was asynchronously received over the connection.
	 * @param message Message to dequeue into.
	 * @return Size of the Message that was dequeued. This includes the size field of the Message. If no Message could be dequeued, this method will return 0.
	 */
	virtual std::size_t Receive( Message& message ) override;

	/** Clear the send and receive queues of this socket.
	 */
	virtual void ClearBuffers() override;

	/** Get the number of bytes queued for sending.
	 * @return Number of bytes queued for sending.
	 */
	virtual std::size_t BytesToSend() const override;

	/** Get the number of bytes queued for receiving.
	 * @return Number of bytes queued for receiving.
	 */
	virtual std::size_t BytesToReceive() const override;

	/** Used to inform subclasses that the transport has connected.
	 */
	virtual void OnConnected() override;

protected:
	/** Ctor.
	 */
	TlsConnection();

	/** Used to inform subclasses that the transport has received data.
	 */
	virtual void OnReceived() override;

	/** Used to inform subclasses that the transport has disconnected.
	 */
	virtual void OnDisconnected() override;

private:
	void OutputCallback( const unsigned char* buffer, std::size_t size );
	void DataCallback( const unsigned char* buffer, std::size_t size );
	void AlertCallback( Botan::TLS::Alert, const unsigned char*, size_t );
	bool HandshakeCallback( const Botan::TLS::Session& );

	const static TlsEndpointType m_type = U;
	const static TlsVerificationType m_verify = V;

	std::vector<char> m_send_buffer = {};
	std::vector<char> m_receive_buffer = {};

	std::array<char, 2048> m_send_memory{ {} };
	std::array<char, 2048> m_receive_memory{ {} };

	std::unique_ptr<Botan::TLS::Channel> m_tls_endpoint{};

	bool m_request_close = false;
	bool m_remote_closed = false;
	bool m_local_closed = false;
};

}

#include <SFNUL/TlsConnection.inl>
