/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <SFNUL/TlsConnection.hpp>
#include <SFNUL/ConfigInternal.hpp>
#include <SFNUL/Concurrency.hpp>
#include <SFNUL/MakeUnique.hpp>
#include <botan/tls_client.h>
#include <botan/tls_server.h>
#include <botan/data_src.h>
#include <botan/pkcs8.h>
#include <botan/auto_rng.h>
#include <botan/init.h>

namespace {

struct TlsCertificateMaker : public sfn::TlsCertificate {};
struct TlsKeyMaker : public sfn::TlsKey {};

class TlsPolicy : public Botan::TLS::Policy {

};

}

namespace sfn {

class TlsResource::LibraryImpl {
public:
	Botan::AutoSeeded_RNG rng;
	TlsPolicy tls_policy;

	static std::weak_ptr<LibraryImpl> library;
};

std::weak_ptr<TlsResource::LibraryImpl> TlsResource::LibraryImpl::library{};

TlsResource::TlsResource() :
	m_library{ LibraryImpl::library.lock() }
{
	if( !m_library ) {
		m_library = std::make_shared<LibraryImpl>();
		LibraryImpl::library = m_library;
	}
}

class TlsCertificate::TlsCertificateImpl {
public:
	TlsCertificateImpl( Botan::X509_Certificate the_certificate ) : certificate{ the_certificate } {}
	Botan::X509_Certificate certificate;
};

TlsCertificate::TlsCertificate() = default;

TlsCertificate::~TlsCertificate() = default;

TlsCertificate::Ptr TlsCertificate::Create( const std::string& certificate ) {
	auto cert_ptr = std::make_shared<TlsCertificateMaker>();
	cert_ptr->LoadCertificate( certificate );
	return cert_ptr;
}

void TlsCertificate::LoadCertificate( const std::string& certificate ) {
	Botan::DataSource_Memory data_source{ certificate };
	m_impl = make_unique<TlsCertificateImpl>( Botan::X509_Certificate( data_source ) );
}

class TlsKey::TlsKeyImpl {
public:
	Botan::Private_Key* key;
};

TlsKey::TlsKey() :
	m_impl{ make_unique<TlsKeyImpl>() }
{
}

TlsKey::~TlsKey() = default;

TlsKey::Ptr TlsKey::Create( const std::string& key, const std::string& password ) {
	auto key_ptr = std::make_shared<TlsKeyMaker>();
	key_ptr->LoadKey( key, password );
	return key_ptr;
}

void TlsKey::LoadKey( const std::string& key, const std::string& password ) {
	Botan::DataSource_Memory data_source{ key };
	m_impl->key = Botan::PKCS8::load_key( data_source, m_library->rng, password );
}

class TlsConnectionBase::TlsConnectionBaseImpl : public Botan::Credentials_Manager, public Botan::TLS::Callbacks {
public:
	TlsConnectionBaseImpl( Botan::RandomNumberGenerator& the_rng ) : session_manager{ the_rng }, rng{ &the_rng } {}

	std::vector<Botan::Certificate_Store*> trusted_certificate_authorities( const std::string& type, const std::string& /*context*/ ) override {
		// We don't support verifying client certificates for now...
		if( type == "tls-server" ) {
			return {};
		}

		std::vector<Botan::Certificate_Store*> trusted_cas{};

		for( auto& ptr : certificate_stores ) {
			trusted_cas.push_back( static_cast<Botan::Certificate_Store*>( ptr.get() ) );
		}

		return trusted_cas;
	}

	std::vector<Botan::X509_Certificate> cert_chain( const std::vector<std::string>& cert_key_types, const std::string& /*type*/, const std::string& /*context*/ ) override {
		// We don't support anything else but RSA for now...
		if( std::find( std::begin( cert_key_types ), std::end( cert_key_types ), "RSA" ) == std::end( cert_key_types ) ) {
			return {};
		}

		if( server_cert && server_cert->m_impl ) {
			std::vector<Botan::X509_Certificate> certificate_chain{ server_cert->m_impl->certificate };

			return certificate_chain;
		}

		return {};
	}

	Botan::Private_Key* private_key_for( const Botan::X509_Certificate& /*cert*/, const std::string& /*type*/, const std::string& /*context*/ ) override {
		if( key && key->m_impl->key ) {
			return key->m_impl->key;
		}

		ErrorMessage() << "No private key loaded for certificate.\n";

		return nullptr;
		// TODO
	}

	Botan::SymmetricKey psk( const std::string& type, const std::string& context, const std::string& identity ) override {
		if( ( type == "tls-server" ) && ( context == "session-ticket" ) ) {
			if( session_ticket_key.length() == 0 ) {
				session_ticket_key = Botan::SymmetricKey( *rng, 32 );
			}
			return session_ticket_key;
		}

		throw Botan::Internal_Error( "No PSK set for " + identity );
		// TODO
	}

	std::string psk_identity( const std::string& /*type*/, const std::string& /*context*/, const std::string& /*identity_hint*/ ) override {
		return "";
		// TODO
	}

	std::string psk_identity_hint( const std::string& /*type*/, const std::string& /*context*/ ) override {
		return "";
		// TODO
	}

	bool attempt_srp( const std::string& /*type*/, const std::string& /*context*/ ) override {
		return false;
		// TODO
	}

	std::string srp_identifier( const std::string& /*type*/, const std::string& /*context*/ ) override {
		return "";
		// TODO
	}

	std::string srp_password( const std::string& /*type*/, const std::string& /*context*/, const std::string& /*identifier*/ ) override {
		return "";
		// TODO
	}

	bool srp_verifier( const std::string& /*type*/, const std::string& /*context*/, const std::string& /*identifier*/, std::string& /*group_name*/, Botan::BigInt& /*verifier*/, std::vector<unsigned char>& /*salt*/, bool /*generate_fake_on_unknown*/ ) override {
		return false;
		// TODO
	}

	void tls_emit_data( const uint8_t data[], size_t size ) override {
		if( output_callback ) {
			output_callback( data, size );
		}
	}

	void tls_record_received( uint64_t, const uint8_t data[], size_t size ) override {
		if( data_callback ) {
			data_callback( data, size );
		}
	}

	void tls_alert( Botan::TLS::Alert alert ) override {
		if( alert_callback ) {
			alert_callback(&alert, nullptr, 0);
		}
	}

	bool tls_session_established( const Botan::TLS::Session& session ) override {
		if( handshake_callback ) {
			return handshake_callback(&session);
		}

		return false;
	}

	void tls_verify_cert_chain( const std::vector<Botan::X509_Certificate>& cert_chain, const std::vector<std::shared_ptr<const Botan::OCSP::Response>>& ocsp_responses, const std::vector<Botan::Certificate_Store*>& trusted_roots, Botan::Usage_Type usage, const std::string& hostname, const Botan::TLS::Policy& policy ) override {
		if( certificate_stores.empty() ) {
			WarningMessage() << "Certificate verification failed: Certificate store empty.\n";
			return;
		}

		last_verification_result = TlsVerificationResult::Passed;

		try {
			Botan::TLS::Callbacks::tls_verify_cert_chain( cert_chain, ocsp_responses, trusted_roots, usage, hostname, policy );
		}
		catch( std::exception& e ) {
			WarningMessage() << "Certificate verification failed: " << e.what() << "\n";

			last_verification_result = TlsVerificationResult::NotTrusted;
		}
	}

	OutputFunction output_callback;
	DataFunction data_callback;
	AlertFunction alert_callback;
	HandshakeFunction handshake_callback;

	std::vector<std::unique_ptr<Botan::Certificate_Store_In_Memory>> certificate_stores;

	TlsVerificationResult last_verification_result{ TlsVerificationResult::NotTrusted };

	Botan::SymmetricKey session_ticket_key;

	Botan::TLS::Session_Manager_In_Memory session_manager;

	TlsCertificate::Ptr server_cert;
	TlsKey::Ptr key;

	std::string common_name;

	Botan::RandomNumberGenerator* rng;

	std::unique_ptr<Botan::TLS::Channel> endpoint;

	std::vector<unsigned char> pre_cert_receive_buffer;
};

TlsConnectionBase::TlsConnectionBase( TlsEndpointType type, TlsVerificationType verify ) :
	m_type{ type },
	m_verify{ verify },
	m_impl{ make_unique<TlsConnectionBaseImpl>( m_library->rng ) }
{

}

TlsConnectionBase::~TlsConnectionBase() = default;

void TlsConnectionBase::AddTrustedCertificate( TlsCertificate::Ptr certificate ) {
	if( m_impl->certificate_stores.empty() ) {
		m_impl->certificate_stores.push_back( make_unique<Botan::Certificate_Store_In_Memory>() );
	}

	if( certificate && certificate->m_impl ) {
		m_impl->certificate_stores.front()->add_certificate( certificate->m_impl->certificate );
	}
}

void TlsConnectionBase::SetPeerCommonName( const std::string& name ) {
	m_impl->common_name = name;
}

void TlsConnectionBase::SetCertificateKeyPair( TlsCertificate::Ptr certificate, TlsKey::Ptr key ) {
	m_impl->server_cert = certificate;
	m_impl->key = key;

	if( !m_impl->pre_cert_receive_buffer.empty() ) {
		m_impl->endpoint->received_data( m_impl->pre_cert_receive_buffer.data(), m_impl->pre_cert_receive_buffer.size() );
		m_impl->pre_cert_receive_buffer.clear();
		m_impl->pre_cert_receive_buffer.shrink_to_fit();
	}
}

TlsVerificationResult TlsConnectionBase::GetVerificationResult() const {
	return m_impl->last_verification_result;
}

/// @cond
void TlsConnectionBase::SetEndpoint( TlsEndpointType type, OutputFunction output_callback, DataFunction data_callback, AlertFunction alert_callback, HandshakeFunction handshake_callback ) {
	m_impl->output_callback = output_callback;
	m_impl->data_callback = data_callback;
	m_impl->alert_callback = alert_callback;
	m_impl->handshake_callback = handshake_callback;

	if( type == TlsEndpointType::Client ) {
		m_impl->endpoint = make_unique<Botan::TLS::Client>(
			*m_impl,
			m_impl->session_manager,
			*m_impl,
			m_library->tls_policy,
			m_library->rng,
			Botan::TLS::Server_Information(),
			Botan::TLS::Protocol_Version::latest_tls_version()
		);
	}
	else if( type == TlsEndpointType::Server ) {
		m_impl->endpoint = make_unique<Botan::TLS::Server>(
			*m_impl,
			m_impl->session_manager,
			*m_impl,
			m_library->tls_policy,
			m_library->rng
		);
	}
}

bool TlsConnectionBase::EndpointIsActive() const {
	return m_impl->endpoint && m_impl->endpoint->is_active();
}

void TlsConnectionBase::CloseEndpoint() {
	if( !m_impl->endpoint ) {
		return;
	}

	m_impl->endpoint->close();
}

void TlsConnectionBase::ResetEndpoint() {
	m_impl->endpoint.reset();
}

void TlsConnectionBase::EndpointSend( const unsigned char* data, std::size_t size ) {
	if( !m_impl->endpoint ) {
		return;
	}

	m_impl->endpoint->send( data, size );
}

void TlsConnectionBase::EndpointReceive( const unsigned char* data, std::size_t size ) {
	if( !m_impl->endpoint ) {
		return;
	}

	if( ( m_type == TlsEndpointType::Server ) && !m_impl->server_cert ) {
		m_impl->pre_cert_receive_buffer.insert( std::end( m_impl->pre_cert_receive_buffer ), data, data + size );
		return;
	}

	m_impl->endpoint->received_data( data, size );
}
/// @endcond

}
