/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <memory>
#include <vector>
#include <type_traits>
#include <tropicssl/net.h>
#include <tropicssl/ssl.h>
#include <tropicssl/certs.h>
#include <tropicssl/havege.h>
#include <tropicssl/x509.h>
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

namespace sf {
class Mutex;
class Packet;
}

namespace sfn {

class ReliableTransport;
class Endpoint;
class Message;

/// @cond
namespace detail {
// Declarations of TropicSSL API we use for declspec
SFNUL_API int ssl_init(ssl_context * ssl);
SFNUL_API void ssl_set_endpoint(ssl_context * ssl, int endpoint);
SFNUL_API void ssl_set_authmode(ssl_context * ssl, int authmode);
SFNUL_API void ssl_set_rng(ssl_context * ssl, int (*f_rng) (void *), void *p_rng);
SFNUL_API void ssl_set_dbg(ssl_context * ssl, void (*f_dbg) (void *, int, const char *), void *p_dbg);
SFNUL_API void ssl_set_bio(ssl_context * ssl, int (*f_recv) (void *, unsigned char *, int), void *p_recv, int (*f_send) (void *, const unsigned char *, int), void *p_send);
SFNUL_API void ssl_set_scb(ssl_context * ssl, int (*s_get) (ssl_context *), int (*s_set) (ssl_context *));
SFNUL_API void ssl_set_session(ssl_context * ssl, int resume, int timeout, ssl_session * session);
SFNUL_API void ssl_set_ciphers(ssl_context * ssl, const int *ciphers);
SFNUL_API void ssl_set_ca_chain(ssl_context * ssl, x509_cert * ca_chain, const char *peer_cn);
SFNUL_API void ssl_set_own_cert(ssl_context * ssl, x509_cert * own_cert, rsa_context * rsa_key);
SFNUL_API int ssl_set_dh_param(ssl_context * ssl, const char *dhm_P, const char *dhm_G);
SFNUL_API int ssl_get_verify_result(const ssl_context * ssl);
SFNUL_API int ssl_handshake(ssl_context * ssl);
SFNUL_API int ssl_read(ssl_context * ssl, unsigned char *buf, int len);
SFNUL_API int ssl_write(ssl_context * ssl, const unsigned char *buf, int len);
SFNUL_API int ssl_close_notify(ssl_context * ssl);
SFNUL_API void ssl_free(ssl_context * ssl);

SFNUL_API void havege_init(havege_state * hs);
SFNUL_API int havege_rand(void *p_rng);
}
/// @endcond

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

template<class T, TlsEndpointType U, TlsVerificationType V> class TlsConnection;

/** TLS Certificate class.
 */
class SFNUL_API TlsCertificate {
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
	/** Ctor.
	 */
	TlsCertificate();

private:
	void LoadCertificate( const std::string& certificate );

	x509_cert m_cert;

	friend class TlsConnectionBase;
};

/** TLS Key class.
 */
class SFNUL_API TlsKey {
public:
	typedef std::shared_ptr<TlsKey> Ptr; //!< Shared pointer.

	/** Dtor.
	 */
	~TlsKey();

	/** Create a TlsKey from a key string and optional password.
	 * @param key std::string containing the Base 64 DER encoded key.
	 * @param password std::string containing the optional password.
	 * @return TlsKey
	 */
	static Ptr Create( const std::string& key, const std::string& password = "" );

protected:
	/** Ctor.
	 */
	TlsKey();

private:
	void LoadKey( const std::string& key, const std::string& password );

	rsa_context m_key;

	friend class TlsConnectionBase;
};

/** TLS connection base.
 */
class SFNUL_API TlsConnectionBase {
public:

	/** Set the Diffie-Hellman generator parameters.
	 * P is set to a pre-computed 1024-value.
	 * G is set to 4.
	 * If you know how to set them yourself, go ahead, you can only gain security if you set them right...
	 * @param p p value as a hexadecimal std::string.
	 * @param g g value as a hexadecimal std::string.
	 */
	static void SetDiffieHellmanParameters( const std::string& p, const std::string& g );

	/** Set the level of debug output produced by the TLS implementation.
	 * 0 to disable, 3 for full debug output. Default: 0
	 * @param level Debug level.
	 */
	void SetDebugLevel( int level );

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
	TlsConnectionBase();
	~TlsConnectionBase();

	virtual void OnSentProxy() = 0;
	virtual void OnReceivedProxy() = 0;

#if defined( __GNUG__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

	ssl_context m_ssl_context{};

#if defined( __GNUG__ )
#pragma GCC diagnostic pop
#endif

	bool require_certificate_key = false;

private:
	static std::string m_diffie_hellman_p;
	static std::string m_diffie_hellman_g;

#if defined( __GNUG__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

	ssl_session m_ssl_session{};

#if defined( __GNUG__ )
#pragma GCC diagnostic pop
#endif

	TlsCertificate::Ptr m_ca_cert{};
	TlsCertificate::Ptr m_server_cert{};
	TlsKey::Ptr m_key{};

	int m_debug_level{ 0 };
	std::string m_common_name = {};
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

	/** Used to inform subclasses that the transport has sent data.
	 */
	virtual void OnSent() override;

	/** Used to inform subclasses that the transport has received data.
	 */
	virtual void OnReceived() override;

	/** Used to inform subclasses that the transport has disconnected.
	 */
	virtual void OnDisconnected() override;

	virtual void OnSentProxy() override;
	virtual void OnReceivedProxy() override;

private:
	int SendInterface( void* /*unused*/, const unsigned char* buffer, int length );
	int RecvInterface( void* /*unused*/, unsigned char* buffer, int length );

	const static TlsEndpointType m_type = U;
	const static TlsVerificationType m_verify = V;

	std::vector<char> m_send_buffer = {};
	std::vector<char> m_receive_buffer = {};

	std::array<char, 2048> m_send_memory{ {} };
	std::array<char, 2048> m_receive_memory{ {} };

	bool m_request_close = false;
	bool m_remote_closed = false;
	bool m_local_closed = false;
};

}

#include <SFNUL/TlsConnection.inl>
