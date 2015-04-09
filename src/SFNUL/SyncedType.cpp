/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <SFNUL/SyncedType.hpp>
#include <SFNUL/SyncedObject.hpp>

namespace {

std::chrono::milliseconds sync_period{ 1000 };

}

namespace sfn {

void SetStreamSynchronizationPeriod( const std::chrono::milliseconds& period ) {
	sync_period = period;
}

std::chrono::milliseconds GetStreamSynchronizationPeriod() {
	return sync_period;
}

BaseSyncedType::BaseSyncedType( SyncedObject* owner, bool stream ) :
	m_owner( owner )
{
	owner->RegisterMember( this );

	if( stream ) {
		owner->EnableStreaming();
	}
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

}
