/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <vector>
#include <chrono>
#include <memory>
#include <SFNUL/Config.hpp>
#include <SFNUL/SyncedType.hpp>

namespace sfn {

class SynchronizerBase;
class SynchronizerServer;
class SynchronizerClient;
class BaseSyncedType;
class Message;

class SFNUL_API SyncedObject {
public:
	typedef Uint32 object_type_id_type;

	/** Move assignment operator.
	 * @param object Object to move from.
	 * @return *this
	 */
	SyncedObject& operator=( SyncedObject&& object );

protected:
	/** Ctor.
	 */
	SyncedObject();

	/** Move Ctor.
	 */
	SyncedObject( SyncedObject&& object );

	/** Dtor.
	 */
	virtual ~SyncedObject();

	virtual object_type_id_type GetTypeID() const = 0;

private:
	friend class BaseSyncedType;
	friend class SynchronizerBase;
	friend class SynchronizerServer;
	friend class SynchronizerClient;

	SyncedObject( const SyncedObject& object ) = delete;

	SyncedObject& operator=( const SyncedObject& object ) = delete;

	typedef Uint32 id_type;
	static const id_type invalid_id = 0;

	static id_type NewID();

	void RegisterMember( BaseSyncedType* member );
	void NotifyChanged();
	void CheckStreamUpdate();

	id_type GetID() const;

	void SetID( id_type id );

	void SetSynchronizer( SynchronizerBase* synchronizer );

	Message Serialize( SynchronizationType sync_type );

	void Deserialize( Message& message );

	std::vector<BaseSyncedType*> m_members;

	SynchronizerBase* m_synchronizer{ nullptr };

	std::unique_ptr<std::chrono::steady_clock::time_point> m_last_stream_sync;

	static id_type m_last_id;

	id_type m_id = invalid_id;

	bool m_changed = true;
};

}
