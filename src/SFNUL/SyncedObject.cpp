/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <SFNUL/SyncedObject.hpp>
#include <SFNUL/Message.hpp>
#include <SFNUL/SyncedType.hpp>
#include <SFNUL/Synchronizer.hpp>
#include <SFNUL/MakeUnique.hpp>

namespace sfn {

SyncedObject::id_type SyncedObject::m_last_id = SyncedObject::invalid_id;

SyncedObject::SyncedObject() {
	m_id = NewID();
}

SyncedObject::SyncedObject( SyncedObject&& object ) {
	m_id = object.m_id;

	if( object.m_synchronizer ) {
		m_synchronizer = object.m_synchronizer;

		object.m_synchronizer->MoveObject( &object, this );

		object.m_synchronizer = nullptr;
	}
}

SyncedObject& SyncedObject::operator=( SyncedObject&& object ) {
	m_id = object.m_id;

	if( m_synchronizer ) {
		if( m_synchronizer == object.m_synchronizer ) {
			m_synchronizer->MoveObject( &object, this );
		}
		else {
			m_synchronizer->RemoveObject( this );

			m_synchronizer = object.m_synchronizer;

			if( m_synchronizer ) {
				m_synchronizer->AddObject( this );
			}
		}
	}

	return *this;
}

SyncedObject::~SyncedObject() {
  if( m_synchronizer ) {
		m_synchronizer->RemoveObject( this );
  }
}

Message SyncedObject::Serialize( SynchronizationType sync_type ) {
	Message message;

	message << static_cast<sfn::Uint8>( sync_type );

	for( auto m : m_members ) {
		m->Serialize( message, sync_type );
	}

	if( m_last_stream_sync ) {
		*m_last_stream_sync = std::chrono::steady_clock::now();
	}

	return message;
}

void SyncedObject::Deserialize( Message& message ) {
	sfn::Uint8 sync_type;

	message >> sync_type;

	for( auto m : m_members ) {
		m->Deserialize( message, static_cast<SynchronizationType>( sync_type ) );
	}
}

SyncedObject::id_type SyncedObject::NewID() {
	return ++m_last_id;
}

SyncedObject::id_type SyncedObject::GetID() const {
	return m_id;
}

void SyncedObject::SetID( SyncedObject::id_type id ) {
	m_id = id;
}

void SyncedObject::RegisterMember( BaseSyncedType* member ) {
	m_members.emplace_back( member );
	m_members.shrink_to_fit();

	if( !m_last_stream_sync && ( member->GetSynchronizationType() == SynchronizationType::Stream ) ) {
		m_last_stream_sync = make_unique<std::chrono::steady_clock::time_point>( std::chrono::steady_clock::now() );
	}
}

void SyncedObject::NotifyChanged() {
	m_changed = true;

	if( m_synchronizer ) {
		m_synchronizer->UpdateObject( this );
	}
}

void SyncedObject::CheckStreamUpdate() {
	if( m_synchronizer && m_last_stream_sync && ( std::chrono::steady_clock::now() - ( *m_last_stream_sync ) > BaseSyncedType::m_sync_period ) ) {
		*m_last_stream_sync = std::chrono::steady_clock::now();
		m_synchronizer->UpdateObject( this );
	}
}

void SyncedObject::SetSynchronizer( SynchronizerBase* synchronizer ) {
	if( m_synchronizer && !m_synchronizer->IsDestroyed() ) {
		m_synchronizer->RemoveObject( this );
	}

	m_synchronizer = synchronizer;

	if( m_synchronizer ) {
		m_synchronizer->AddObject( this );
	}
}

}
