#pragma once

#include <memory>
#include <type_traits>
#include <tropicssl/net.h>
#include <tropicssl/ssl.h>
#include <tropicssl/certs.h>
#include <tropicssl/havege.h>
#include <tropicssl/x509.h>
#include <SFNUL/Config.hpp>
#include <SFNUL/ReliableTransport.hpp>

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

	template<class T, TlsEndpointType U, TlsVerificationType V>
	friend class TlsConnection;
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

	template<class T, TlsEndpointType U, TlsVerificationType V>
	friend class TlsConnection;
};

#if defined( __GNUG__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#endif

/** TLS connection class.
 */
template<class T, TlsEndpointType U, TlsVerificationType V>
class SFNUL_API TlsConnection : public T {

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

	/** Set the Diffie-Hellman generator parameters.
	 * P is set to a pre-computed 1024-value.
	 * G is set to 4.
	 * If you know how to set them yourself, go ahead, you can only gain security if you set them right...
	 * @param p p value as a hexadecimal std::string.
	 * @param g g value as a hexadecimal std::string.
	 */
	static void SetDiffieHellmanParameters( const std::string& p, const std::string& g );

	/** Get the result of the certificate verification.
	 * @return Bitwise ORed verification flags.
	 */
	TlsVerificationResult GetVerificationResult() const;

	/** Asynchronously connect to a remote endpoint.
	 * @param endpoint Remote endpoint.
	 */
	virtual void Connect( const Endpoint& endpoint );

	/** Shutdown the connection for sending. This is required for graceful connection termination.
	 */
	virtual void Shutdown();

	/** Check if the local system has shut the connection down for sending.
	 * @return true if the local system has shut the connection down for sending.
	 */
	virtual bool LocalHasShutdown() const;

	/** Check if the remote system has shut the connection down for sending.
	 * @return true if the remote system has shut the connection down for sending.
	 */
	virtual bool RemoteHasShutdown() const;

	/** Check if the TLS connection is operational.
	 * @return true if the TLS connection is operational.
	 */
	virtual bool IsConnected() const;

	/** Close the connection. This frees up the operating system resources assigned to the connection.
	 */
	virtual void Close();

	/** Queue data up for asynchronous sending over the connection.
	 * @param data Pointer to a block of memory containing the data to queue.
	 * @param size Size of the block of memory containing the data to queue.
	 */
	virtual void Send( const void* data, std::size_t size );

	/** Dequeue data that was asynchronously received over the connection.
	 * @param data Pointer to a block of memory that will contain the data to dequeue.
	 * @param size Size of the block of memory that will contain the data to dequeue.
	 * @return Number of bytes of data that was actually dequeued.
	 */
	virtual std::size_t Receive( void* data, std::size_t size );

	/** Queue an sf::Packet up for asynchronous sending over the connection.
	 * @param packet sf::Packet to queue.
	 */
	virtual void Send( sf::Packet& packet );

	/** Dequeue an sf::Packet that was asynchronously received over the connection.
	 * @param packet sf::Packet to dequeue into.
	 * @return Size of the sf::Packet that was dequeued. This includes the size field of the packet. If no packet could be dequeued, this method will return 0.
	 */
	virtual std::size_t Receive( sf::Packet& packet );

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

private:
	int SendInterface( void* /*unused*/, const unsigned char* buffer, int length );
	int RecvInterface( void* /*unused*/, unsigned char* buffer, int length );

	static std::string m_diffie_hellman_p;
	static std::string m_diffie_hellman_g;

	static bool havege_initialized;
	static havege_state m_havege_state;

	ssl_context m_ssl_context;
	ssl_session m_ssl_session;
	TlsCertificate::Ptr m_ca_cert;
	TlsCertificate::Ptr m_server_cert;
	TlsKey::Ptr m_key;

	int m_debug_level;

	const static TlsEndpointType m_type = U;
	const static TlsVerificationType m_verify = V;

	std::vector<char> m_send_buffer = {};
	std::vector<char> m_receive_buffer = {};

	std::array<char, 2048> m_send_memory;
	std::array<char, 2048> m_receive_memory;

	std::string m_common_name = {};

	/// @cond
	mutable sf::Mutex m_mutex;
	/// @endcond

	bool m_request_close = false;
	bool m_remote_closed = true;
	bool m_local_closed = true;

	bool require_certificate_key = false;
};

}

#include <SFNUL/TlsConnection.inl>
