/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <SFNUL/TlsConnection.hpp>
#include <SFNUL/Concurrency.hpp>

namespace sfn {

std::weak_ptr<Botan::LibraryInitializer> BotanResource::m_library_initializer{};

BotanResource::BotanResource() :
	m_library_initializer_reference{ m_library_initializer.lock() }
{
	if( !m_library_initializer_reference ) {
		m_library_initializer_reference = std::make_shared<Botan::LibraryInitializer>();
		m_library_initializer = m_library_initializer_reference;
	}
}

namespace {
struct TlsCertificateMaker : public TlsCertificate {};
struct TlsKeyMaker : public TlsKey {};
}

TlsCertificate::Ptr TlsCertificate::Create( const std::string& certificate ) {
	auto cert_ptr = std::make_shared<TlsCertificateMaker>();
	cert_ptr->LoadCertificate( certificate );
	return cert_ptr;
}

void TlsCertificate::LoadCertificate( const std::string& certificate ) {
	Botan::DataSource_Memory data_source{ certificate };
	m_certificate = std::make_unique<Botan::X509_Certificate>( data_source );
}

TlsKey::Ptr TlsKey::Create( const std::string& key, const std::string& password ) {
	auto key_ptr = std::make_shared<TlsKeyMaker>();
	key_ptr->LoadKey( key, password );
	return key_ptr;
}

void TlsKey::LoadKey( const std::string& key, const std::string& password ) {
	Botan::DataSource_Memory data_source{ key };
	m_key = Botan::PKCS8::load_key( data_source, m_rng, password );
}

void TlsConnectionBase::AddTrustedCertificate( TlsCertificate::Ptr certificate ) {
	if( m_certificate_stores.empty() ) {
		m_certificate_stores.push_back( std::make_unique<Botan::Certificate_Store_In_Memory>() );
	}

	if( certificate && certificate->m_certificate ) {
		static_cast<Botan::Certificate_Store_In_Memory*>( m_certificate_stores.front().get() )->add_certificate( *( certificate->m_certificate ) );
	}
}

void TlsConnectionBase::SetPeerCommonName( const std::string& name ) {
	m_common_name = name;
}

void TlsConnectionBase::SetCertificateKeyPair( TlsCertificate::Ptr certificate, TlsKey::Ptr key ) {
	m_server_cert = certificate;
	m_key = key;

	if( require_certificate_key ) {
		require_certificate_key = false;
	}
}

TlsVerificationResult TlsConnectionBase::GetVerificationResult() const {
	return m_last_verification_result;
}

std::vector<Botan::Certificate_Store*> TlsConnectionBase::trusted_certificate_authorities( const std::string& type, const std::string& /*context*/ ) {
	// We don't support verifying client certificates for now...
	if( type == "tls-server" ) {
		return {};
	}

	std::vector<Botan::Certificate_Store*> trusted_cas{};

	for( auto& ptr : m_certificate_stores ) {
		trusted_cas.push_back( static_cast<Botan::Certificate_Store*>( ptr.get() ) );
	}

	return trusted_cas;
}

void TlsConnectionBase::verify_certificate_chain( const std::string& type, const std::string& hostname, const std::vector<Botan::X509_Certificate>& certificate_chain ) {
	if( m_certificate_stores.empty() ) {
		WarningMessage() << "Certificate verification failed: Certificate store empty.\n";
		return;
	}

	m_last_verification_result = TlsVerificationResult::PASSED;

	try {
		Botan::Credentials_Manager::verify_certificate_chain( type, hostname, certificate_chain );
	}
	catch( std::exception& e ) {
		WarningMessage() << "Certificate verification failed: " << e.what() << "\n";

		m_last_verification_result = TlsVerificationResult::NOT_TRUSTED;
	}
}

std::vector<Botan::X509_Certificate> TlsConnectionBase::cert_chain( const std::vector<std::string>& cert_key_types, const std::string& /*type*/, const std::string& /*context*/ ) {
	// We don't support anything else but RSA for now...
	if( std::find( std::begin( cert_key_types ), std::end( cert_key_types ), "RSA" ) == std::end( cert_key_types ) ) {
		return {};
	}

	if( m_server_cert && m_server_cert->m_certificate ) {
		std::vector<Botan::X509_Certificate> certificate_chain{ *( m_server_cert->m_certificate ) };

		return certificate_chain;
	}

	return {};
}

Botan::Private_Key* TlsConnectionBase::private_key_for( const Botan::X509_Certificate& /*cert*/, const std::string& /*type*/, const std::string& /*context*/ ) {
	if( m_key && m_key->m_key ) {
		return m_key->m_key;
	}

	ErrorMessage() << "No private key loaded for certificate.\n";

	return nullptr;
	// TODO
}

Botan::SymmetricKey TlsConnectionBase::psk( const std::string& type, const std::string& context, const std::string& identity ) {
	if( ( type == "tls-server" ) && ( context == "session-ticket" ) ) {
		if( m_session_ticket_key.length() == 0 ) {
			m_session_ticket_key = Botan::SymmetricKey( m_rng, 32 );
		}
		return m_session_ticket_key;
	}

	throw Botan::Internal_Error( "No PSK set for " + identity );
	// TODO
}

std::string TlsConnectionBase::psk_identity( const std::string& /*type*/, const std::string& /*context*/, const std::string& /*identity_hint*/ ) {
	return "";
	// TODO
}

std::string TlsConnectionBase::psk_identity_hint( const std::string& /*type*/, const std::string& /*context*/ ) {
	return "";
	// TODO
}

bool TlsConnectionBase::attempt_srp( const std::string& /*type*/, const std::string& /*context*/ ) {
	return false;
	// TODO
}

std::string TlsConnectionBase::srp_identifier( const std::string& /*type*/, const std::string& /*context*/ ) {
	return "";
	// TODO
}

std::string TlsConnectionBase::srp_password( const std::string& /*type*/, const std::string& /*context*/, const std::string& /*identifier*/ ) {
	return "";
	// TODO
}

bool TlsConnectionBase::srp_verifier( const std::string& /*type*/, const std::string& /*context*/, const std::string& /*identifier*/, std::string& /*group_name*/, Botan::BigInt& /*verifier*/, std::vector<unsigned char>& /*salt*/, bool /*generate_fake_on_unknown*/ ) {
	return false;
	// TODO
}

}
