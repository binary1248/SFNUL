/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <iostream>
#include <cassert>
#include <algorithm>
#include <SFNUL/Synchronizer.hpp>
#include <SFNUL/TcpSocket.hpp>
#include <SFNUL/Link.hpp>
#include <SFNUL/Message.hpp>

// Until C++14 comes along...
namespace std {
template<typename T, typename... Args>
inline std::unique_ptr<T> make_unique( Args&&... args ) {
	return std::unique_ptr<T>( new T( std::forward<Args>( args )... ) );
}
}

namespace sfn {

const Link<ReliableTransport>::stream_id_type SynchronizerBase::stream_id = 200;

SyncedObject* SynchronizerBase::GetObjectByID( SyncedObject::id_type id ) {
	auto iter = std::find_if( std::begin( m_objects ), std::end( m_objects ),
		[id]( SyncedObject* object ) {
			return object->GetID() == id;
		}
	);

	if( iter == std::end( m_objects ) ) {
		std::cerr << "Failed to find object " << id << "\n";
	}

	assert( iter != std::end( m_objects ) );
	assert( *iter );

	return *iter;
}

void SynchronizerBase::MoveObject( SyncedObject* object_old, SyncedObject* object_new ) {
	if( object_old != object_new ) {
		auto iter = std::find( std::begin( m_objects ), std::end( m_objects ), object_old );

		if( iter != std::end( m_objects ) ) {
			( *iter ) = object_new;
		}
	}
}

bool SynchronizerBase::IsDestroyed() const {
	return m_destroyed;
}

SynchronizerServer::SynchronizerServer() {
}

SynchronizerServer::~SynchronizerServer() {
	m_destroyed = true;

	for( auto o : m_objects ) {
		o->SetSynchronizer( nullptr );
	}
}

SynchronizerServer::SynchronizerServer( SynchronizerServer&& synchronizer ) {
	std::swap( m_objects, synchronizer.m_objects );

	for( auto o : m_objects ) {
		o->SetSynchronizer( this );
	}
}

SynchronizerServer& SynchronizerServer::operator=( SynchronizerServer&& synchronizer ) {
	std::swap( m_objects, synchronizer.m_objects );

	for( auto o : m_objects ) {
		o->SetSynchronizer( this );
	}

	return *this;
}

bool SynchronizerServer::AddClient( std::weak_ptr<Link<TcpSocket>> client_link ) {
	auto shared_client_link = client_link.lock();

	if( !shared_client_link ) {
		return false;
	}

	auto iter = std::find_if( std::begin( m_links ), std::end( m_links ),
		[&shared_client_link]( const std::weak_ptr<Link<TcpSocket>>& element ) {
			// We don't need to check if they are both 0 since we get rid of those while updating.
			return shared_client_link == element.lock();
		}
	);

	if( iter == std::end( m_links ) ) {
		m_links.emplace_back( client_link );

		for( auto o : m_objects ) {
			auto message = o->Serialize();
			o->GetID() >> message;
			o->GetTypeID() >> message;
			sync_type::create >> message;

			shared_client_link->Send( stream_id, message );
		}

		return true;
	}

	return false;
}

bool SynchronizerServer::RemoveClient( std::weak_ptr<Link<TcpSocket>> client_link ) {
	auto shared_client_link = client_link.lock();

	if( !shared_client_link ) {
		return false;
	}

	auto iter = std::find_if( std::begin( m_links ), std::end( m_links ),
		[&shared_client_link]( const std::weak_ptr<Link<TcpSocket>>& element ) {
			// We don't need to check if they are both 0 since we get rid of those while updating.
			return shared_client_link == element.lock();
		}
	);

	if( iter != std::end( m_links ) ) {
		m_links.erase( iter );
		return true;
	}

	return false;
}

void SynchronizerServer::Update() {
	Send();
}

bool SynchronizerServer::AddObject( SyncedObject* object ) {
	if( object->GetID() == SyncedObject::invalid_id ) {
		return false;
	}

	auto iter = std::find( std::begin( m_objects ), std::end( m_objects ), object );

	if( iter == std::end( m_objects ) ) {
		m_objects.emplace_back( object );
		m_updates[object->GetID()] = sync_type::create;
		return true;
	}

	return false;
}

void SynchronizerServer::UpdateObject( SyncedObject* object ) {
	auto iter = m_updates.find( object->GetID() );

	if( ( iter == std::end( m_updates ) ) || ( iter->second != sync_type::create ) ) {
		m_updates[object->GetID()] = sync_type::update;
	}
}

bool SynchronizerServer::RemoveObject( SyncedObject* object ) {
	if( object->GetID() == SyncedObject::invalid_id ) {
		return false;
	}

	auto iter = std::find( std::begin( m_objects ), std::end( m_objects ), object );

	if( iter != std::end( m_objects ) ) {
		m_objects.erase( iter );

		auto update_object = m_updates.find( object->GetID() );

		if( update_object != std::end( m_updates ) ) {
			if( update_object->second == sync_type::create ) {
				m_updates.erase( update_object );
			}
			else if( update_object->second == sync_type::update ) {
				update_object->second = sync_type::destroy;
			}
		}
		else {
			m_updates.emplace( object->GetID(), sync_type::destroy );
		}

		return true;
	}

	return false;
}

void SynchronizerServer::Send() {
	for( const auto& u : m_updates ) {
		if( u.first == SyncedObject::invalid_id ) {
			continue;
		}

		Message message;

		if( u.second == sync_type::create ) {
			auto object = GetObjectByID( u.first );
			message = object->Serialize();
			u.first >> message;
			object->GetTypeID() >> message;
			sync_type::create >> message;
		}
		else if( u.second == sync_type::update ) {
			message = GetObjectByID( u.first )->Serialize();
			u.first >> message;
			sync_type::update >> message;
		}
		else if( u.second == sync_type::destroy ) {
			u.first >> message;
			sync_type::destroy >> message;
		}

		for( auto iter = std::begin( m_links ); iter != std::end( m_links ); ) {
			auto client_link = iter->lock();

			if( !client_link || !client_link->GetTransport() || !client_link->GetTransport()->IsConnected() || client_link->GetTransport()->RemoteHasShutdown() || client_link->GetTransport()->LocalHasShutdown() ) {
				iter = m_links.erase( iter );
				continue;
			}

			client_link->Send( stream_id, message );

			++iter;
		}
	}

	m_updates.clear();
}

SynchronizerClient::SynchronizerClient() {
}

SynchronizerClient::~SynchronizerClient() {
	m_destroyed = true;

	for( auto o : m_objects ) {
		o->SetSynchronizer( nullptr );
	}
}

SynchronizerClient::SynchronizerClient( SynchronizerClient&& synchronizer ) {
	std::swap( m_objects, synchronizer.m_objects );

	for( auto o : m_objects ) {
		o->SetSynchronizer( this );
	}
}

SynchronizerClient& SynchronizerClient::operator=( SynchronizerClient&& synchronizer ) {
	std::swap( m_objects, synchronizer.m_objects );

	for( auto o : m_objects ) {
		o->SetSynchronizer( this );
	}

	return *this;
}

bool SynchronizerClient::AddServer( std::weak_ptr<Link<TcpSocket>> server_link ) {
	auto shared_server_link = server_link.lock();

	if( !shared_server_link ) {
		return false;
	}

	auto iter = std::find_if( std::begin( m_links ), std::end( m_links ),
		[&shared_server_link]( const std::weak_ptr<Link<TcpSocket>>& element ) {
			// We don't need to check if they are both 0 since we get rid of those while updating.
			return shared_server_link == element.lock();
		}
	);

	if( iter == std::end( m_links ) ) {
		m_links.emplace_back( server_link );
		return true;
	}

	return false;
}

bool SynchronizerClient::RemoveServer( std::weak_ptr<Link<TcpSocket>> server_link ) {
	auto shared_server_link = server_link.lock();

	if( !shared_server_link ) {
		return false;
	}

	auto iter = std::find_if( std::begin( m_links ), std::end( m_links ),
		[&shared_server_link]( const std::weak_ptr<Link<TcpSocket>>& element ) {
			// We don't need to check if they are both 0 since we get rid of those while updating.
			return shared_server_link == element.lock();
		}
	);

	if( iter != std::end( m_links ) ) {
		m_links.erase( iter );
		return true;
	}

	return false;
}

void SynchronizerClient::Update() {
	Receive();
}

bool SynchronizerClient::AddObject( SyncedObject* object ) {
	if( object->GetID() == SyncedObject::invalid_id ) {
		return false;
	}

	auto iter = std::find( std::begin( m_objects ), std::end( m_objects ), object );

	if( iter == std::end( m_objects ) ) {
		m_objects.emplace_back( object );
		return true;
	}

	return false;
}

void SynchronizerClient::UpdateObject( SyncedObject* /*object*/ ) {
}

bool SynchronizerClient::RemoveObject( SyncedObject* object ) {
	if( object->GetID() == SyncedObject::invalid_id ) {
		return false;
	}

	auto iter = std::find( std::begin( m_objects ), std::end( m_objects ), object );

	if( iter != std::end( m_objects ) ) {
		m_objects.erase( iter );
		return true;
	}

	return false;
}

void SynchronizerClient::SetLifetimeManagers( SyncedObject::object_type_id_type type_id, std::function<SyncedObject*()> factory, std::function<void( SyncedObject* )> destructor ) {
	m_factories[type_id] = factory;
	m_destructors[type_id] = destructor;
}

void SynchronizerClient::Receive() {
	for( auto iter = std::begin( m_links ); iter != std::end( m_links ); ) {
		auto server = iter->lock();

		if( !server || !server->GetTransport() || !server->GetTransport()->IsConnected() || server->GetTransport()->RemoteHasShutdown() || server->GetTransport()->LocalHasShutdown() ) {
			iter = m_links.erase( iter );
			continue;
		}

		std::size_t received = 0;
		Message message;

		do {
			received = server->Receive( stream_id, message );

			if( received ) {
				if( message.GetSize() < sizeof( sync_type ) + sizeof( SyncedObject::id_type ) ) {
					std::cerr << "Invalid Synchronizer message received by the client.\n";
					continue;
				}

				sync_type update_type;
				message >> update_type;

				switch( update_type ) {
					case sync_type::create: {
						SyncedObject::object_type_id_type type;
						message >> type;

						if( !m_factories[type] ) {
							std::cerr << "Error: No factories registered for type " << type << ".\n";
						}

						assert( m_factories[type] );

						SyncedObject::id_type id;
						message >> id;
						assert( id != SyncedObject::invalid_id );

						SyncedObject* object = m_factories[type]();
						assert( object );
						object->SetSynchronizer( this );
						object->SetID( id );
						assert( object == GetObjectByID( id ) );
						object->Deserialize( message );

						break;
					}
					case sync_type::update: {
						SyncedObject::id_type id;
						message >> id;
						assert( id != SyncedObject::invalid_id );

						auto object = GetObjectByID( id );
						assert( object );

						object->Deserialize( message );

						break;
					}
					case sync_type::destroy: {
						SyncedObject::id_type id;
						message >> id;
						assert( id != SyncedObject::invalid_id );

						auto object = GetObjectByID( id );
						assert( object );

						auto type = object->GetTypeID();

						if( !m_destructors[type] ) {
							std::cerr << "Error: No destructors registered for type " << type << ".\n";
						}

						assert( m_destructors[type] );
						m_destructors[type]( object );

						break;
					}
					default: {
						std::cerr << "Warning: Invalid Synchronizer synchronization type specified by the server.\n";
						break;
					}
				}
			}
		} while( received );

		++iter;
	}
}

}
