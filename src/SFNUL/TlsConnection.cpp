#include <SFNUL/TlsConnection.hpp>

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
		std::cerr << "TlsCertificate::Create() Error: x509parse_crt returned: " << result << "\n";
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
		std::cerr << "SetCertificateKeyPair() Error: x509parse_key returned: " << result << "\n";
	}
}

}
