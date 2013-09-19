#include <SFNUL/TlsConnection.hpp>

namespace sfn {

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
	auto result = x509parse_crt( &m_cert, reinterpret_cast<const unsigned char*>( certificate.c_str() ), certificate.length() );

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
	auto result = x509parse_key( &m_key, reinterpret_cast<const unsigned char*>( key.c_str() ), key.length(), password.empty() ? nullptr : reinterpret_cast<const unsigned char*>( password.c_str() ), password.length() );

	if( result < 0 ) {
		std::cerr << "SetCertificateKeyPair() Error: x509parse_key returned: " << result << "\n";
	}
}

}
