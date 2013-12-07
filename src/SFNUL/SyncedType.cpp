/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <SFNUL/SyncedType.hpp>
#include <SFNUL/SyncedObject.hpp>

namespace sfn {

SFNUL_API void SetStreamSynchronizationPeriod( const std::chrono::milliseconds& period ) {
	BaseSyncedType::m_sync_period = period;
}

std::chrono::milliseconds BaseSyncedType::m_sync_period{ 1000 };

BaseSyncedType::BaseSyncedType( SyncedObject* owner, SynchronizationType sync_type ) :
	m_sync_type( sync_type ),
	m_owner( owner )
{
	owner->RegisterMember( this );
}

BaseSyncedType::~BaseSyncedType() {
}

BaseSyncedType& BaseSyncedType::operator=( const BaseSyncedType& /*other*/ ) {
	return *this;
}

BaseSyncedType& BaseSyncedType::operator=( BaseSyncedType&& /*other*/ ) {
	return *this;
}

bool BaseSyncedType::GetModified() const {
	return m_modified;
}

void BaseSyncedType::SetModified( bool modified ) {
	m_modified = modified;

	m_owner->NotifyChanged();
}

SynchronizationType BaseSyncedType::GetSynchronizationType() const {
	return m_sync_type;
}

}
