/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <SFNUL/TlsConnection.hpp>
#include <SFNUL/Concurrency.hpp>

namespace sfn {

/// @cond
namespace detail {
int ssl_init(ssl_context * ssl) { return ::ssl_init( ssl ); }
void ssl_set_endpoint(ssl_context * ssl, int endpoint) { ::ssl_set_endpoint( ssl, endpoint ); }
void ssl_set_authmode(ssl_context * ssl, int authmode) { ::ssl_set_authmode( ssl, authmode ); }
void ssl_set_rng(ssl_context * ssl, int (*f_rng) (void *), void *p_rng) { ::ssl_set_rng( ssl, f_rng, p_rng ); }
void ssl_set_dbg(ssl_context * ssl, void (*f_dbg) (void *, int, const char *), void *p_dbg) { ::ssl_set_dbg( ssl, f_dbg, p_dbg ); }
void ssl_set_bio(ssl_context * ssl, int (*f_recv) (void *, unsigned char *, int), void *p_recv, int (*f_send) (void *, const unsigned char *, int), void *p_send) { ::ssl_set_bio( ssl, f_recv, p_recv, f_send, p_send ); }
void ssl_set_scb(ssl_context * ssl, int (*s_get) (ssl_context *), int (*s_set) (ssl_context *)) { ::ssl_set_scb( ssl, s_get, s_set ); }
void ssl_set_session(ssl_context * ssl, int resume, int timeout, ssl_session * session) { ::ssl_set_session( ssl, resume, timeout, session ); }
void ssl_set_ciphers(ssl_context * ssl, const int *ciphers) { ::ssl_set_ciphers( ssl, ciphers ); }
void ssl_set_ca_chain(ssl_context * ssl, x509_cert * ca_chain, const char *peer_cn) { ::ssl_set_ca_chain( ssl, ca_chain, peer_cn ); }
void ssl_set_own_cert(ssl_context * ssl, x509_cert * own_cert, rsa_context * rsa_key) { ::ssl_set_own_cert( ssl, own_cert, rsa_key ); }
int ssl_set_dh_param(ssl_context * ssl, const char *dhm_P, const char *dhm_G) { return ::ssl_set_dh_param( ssl, dhm_P, dhm_G ); }
int ssl_get_verify_result(const ssl_context * ssl) { return ::ssl_get_verify_result( ssl ); }
int ssl_handshake(ssl_context * ssl) { return ::ssl_handshake( ssl ); }
int ssl_read(ssl_context * ssl, unsigned char *buf, int len) { return ::ssl_read( ssl, buf, len ); }
int ssl_write(ssl_context * ssl, const unsigned char *buf, int len) { return ::ssl_write( ssl, buf, len ); }
int ssl_close_notify(ssl_context * ssl) { return ::ssl_close_notify( ssl ); }
void ssl_free(ssl_context * ssl) { ::ssl_free( ssl ); }

void havege_init(havege_state * hs) { ::havege_init( hs ); }
int havege_rand(void *p_rng) { return ::havege_rand( p_rng ); }

#if defined( __GNUG__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#endif

class SFNUL_API Havege : public Atomic {

#if defined( __GNUG__ )
#pragma GCC diagnostic pop
#endif

public:
	Havege();

	int Rand();
private:

#if defined( __GNUG__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

	havege_state m_havege_state{};

#if defined( __GNUG__ )
#pragma GCC diagnostic pop
#endif

};

}
/// @endcond

#if defined( __GNUG__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#endif

namespace {
struct TlsCertificateMaker : public TlsCertificate {};
struct TlsKeyMaker : public TlsKey {};
}

#if defined( __GNUG__ )
#pragma GCC diagnostic pop
#endif

std::string TlsConnectionBase::m_diffie_hellman_p =
"9272A172ADD2399D00BAE09B93FF020099D79C481561F9F1003DEADF17F58C93"
"85A3F21F01B6617A69D0FB1EF0F778F7B9E88A19EFC6453CFC29D61E5A7589A7"
"F11BB853EA1FB25769693F703BA720198BEA1C79EB7A718215E88391FAF5B1E5"
"AC0D60080C0C65C3534903119B845B2EE75F322D98EBB7A660C6A9EAB633D02B";

std::string TlsConnectionBase::m_diffie_hellman_g =
"04";

sfn::detail::Havege::Havege() {
	auto lock = AcquireLock();

	sfn::detail::havege_init( &m_havege_state );
}

int sfn::detail::Havege::Rand() {
	auto lock = AcquireLock();

	return sfn::detail::havege_rand( &m_havege_state );
}

void TlsConnectionBase::SetDiffieHellmanParameters( const std::string& p, const std::string& g ) {
	m_diffie_hellman_p = p;
	m_diffie_hellman_g = g;
}

TlsCertificate::Ptr TlsCertificate::Create( const std::string& certificate ) {
	auto cert_ptr = std::make_shared<TlsCertificateMaker>();
	cert_ptr->LoadCertificate( certificate );
	return cert_ptr;
}

TlsCertificate::TlsCertificate() :
	m_cert()
{
	std::memset( &m_cert, 0, sizeof( x509_cert ) );
}

TlsCertificate::~TlsCertificate() {
	x509_free( &m_cert );
}

void TlsCertificate::LoadCertificate( const std::string& certificate ) {
	auto result = x509parse_crt( &m_cert, reinterpret_cast<const unsigned char*>( certificate.c_str() ), static_cast<int>( certificate.length() ) );

	if( result < 0 ) {
		ErrorMessage() << "TlsCertificate::Create() Error: x509parse_crt returned: " << result << "\n";
	}
}

TlsKey::Ptr TlsKey::Create( const std::string& key, const std::string& password ) {
	auto key_ptr = std::make_shared<TlsKeyMaker>();
	key_ptr->LoadKey( key, password );
	return key_ptr;
}

TlsKey::TlsKey() :
	m_key()
{
	std::memset( &m_key, 0, sizeof( rsa_context ) );
}

TlsKey::~TlsKey() {
	rsa_free( &m_key );
}

void TlsKey::LoadKey( const std::string& key, const std::string& password ) {
	auto result = x509parse_key( &m_key, reinterpret_cast<const unsigned char*>( key.c_str() ), static_cast<int>( key.length() ), password.empty() ? nullptr : reinterpret_cast<const unsigned char*>( password.c_str() ), static_cast<int>( password.length() ) );

	if( result < 0 ) {
		ErrorMessage() << "SetCertificateKeyPair() Error: x509parse_key returned: " << result << "\n";
	}
}

TlsConnectionBase::TlsConnectionBase() {
	std::memset( &m_ssl_context, 0, sizeof( ssl_context ) );
	std::memset( &m_ssl_session, 0, sizeof( ssl_session ) );

	auto result = sfn::detail::ssl_init( &m_ssl_context );

	if( result ) {
		ErrorMessage() << "TlsConnection() Error: ssl_init returned: " << result << "\n";
		return;
	}

	sfn::detail::ssl_set_rng(
		&m_ssl_context,
		[]( void* /*unused*/ ) {
			static sfn::detail::Havege havege;

			return havege.Rand();
		},
		nullptr
	);

	static const int ciphers[] = {
		SSL_EDH_RSA_AES_256_SHA,
		SSL_EDH_RSA_CAMELLIA_256_SHA,
		SSL_EDH_RSA_DES_168_SHA,
		SSL_RSA_AES_256_SHA,
		SSL_RSA_CAMELLIA_256_SHA,
		SSL_RSA_AES_128_SHA,
		SSL_RSA_CAMELLIA_128_SHA,
		SSL_RSA_DES_168_SHA,
		SSL_RSA_RC4_128_SHA,
		SSL_RSA_RC4_128_MD5,
		0
	};

	sfn::detail::ssl_set_ciphers( &m_ssl_context, ciphers );

	sfn::detail::ssl_set_session( &m_ssl_context, 1, 0, &m_ssl_session );

	sfn::detail::ssl_set_dh_param( &m_ssl_context, m_diffie_hellman_p.c_str(), m_diffie_hellman_g.c_str() );

	// Disable session resumption for the time being.
	// TODO: Add session resumption support.
	sfn::detail::ssl_set_scb( &m_ssl_context, []( ssl_context* ) { return 1; }, []( ssl_context* ) { return 1; } );

	sfn::detail::ssl_set_dbg(
		&m_ssl_context,
		[]( void* context, int ssl_dbg_message_level, const char* debug_message ) {
			if( ssl_dbg_message_level <= *static_cast<int*>( context ) ) {
				if(

					!std::strstr( debug_message, "returned -3984 (0xfffff070)" ) &&

					// Don't inform about function completions, we don't encounter hangs...
					!std::strstr( debug_message, "<=" ) &&

					!std::strstr( debug_message, "handshake\n" ) &&
					!std::strstr( debug_message, "ssl->f_send()" ) &&
					!std::strstr( debug_message, "ssl->f_recv()" ) &&
					!std::strstr( debug_message, "flush output" ) &&
					!std::strstr( debug_message, "fetch input" ) &&
					!std::strstr( debug_message, "in_left" ) &&
					!std::strstr( debug_message, "out_left" ) &&
					!std::strstr( debug_message, "=> read\n" ) &&
					!std::strstr( debug_message, "<= read\n" ) &&
					!std::strstr( debug_message, "=> write\n" ) &&
					!std::strstr( debug_message, "<= write\n" ) &&

					true
				) {
					DebugMessage() << debug_message;
				}
			}
		},
		&m_debug_level
	);
}

TlsConnectionBase::~TlsConnectionBase() {
	sfn::detail::ssl_free( &m_ssl_context );

	memset( &m_ssl_context, 0, sizeof( ssl_context ) );
}

void TlsConnectionBase::SetDebugLevel( int level ) {
	m_debug_level = level;
}

void TlsConnectionBase::AddTrustedCertificate( TlsCertificate::Ptr certificate ) {
	m_ca_cert = certificate;

	// We don't support client certificate authentication yet.
	// TODO: Add client certificate authentication.
	sfn::detail::ssl_set_ca_chain( &m_ssl_context, &( m_ca_cert->m_cert ), nullptr );
}

void TlsConnectionBase::SetPeerCommonName( const std::string& name ) {
	m_common_name = name;
}

void TlsConnectionBase::SetCertificateKeyPair( TlsCertificate::Ptr certificate, TlsKey::Ptr key ) {
	m_server_cert = certificate;
	m_key = key;

	sfn::detail::ssl_set_own_cert( &m_ssl_context, &( m_server_cert->m_cert ), &( m_key->m_key ) );

	// For now we only support self-signed certificates.
	// TODO: Add proper certificate chain support for servers.
	sfn::detail::ssl_set_ca_chain( &m_ssl_context, &( m_server_cert->m_cert ), nullptr );

	if( require_certificate_key ) {
		require_certificate_key = false;

		OnSentProxy();
		OnReceivedProxy();
	}
}

TlsVerificationResult TlsConnectionBase::GetVerificationResult() const {
	auto verify_result = sfn::detail::ssl_get_verify_result( &m_ssl_context );

	TlsVerificationResult result = TlsVerificationResult::PASSED;

	if( verify_result & BADCERT_EXPIRED ) {
		result |= TlsVerificationResult::EXPIRED;
	}

	if( verify_result & BADCERT_REVOKED ) {
		result |= TlsVerificationResult::REVOKED;
	}

	if( verify_result & BADCERT_CN_MISMATCH ) {
		result |= TlsVerificationResult::CN_MISMATCH;
	}

	if( verify_result & BADCERT_NOT_TRUSTED ) {
		result |= TlsVerificationResult::NOT_TRUSTED;
	}

	return result;
}

}
