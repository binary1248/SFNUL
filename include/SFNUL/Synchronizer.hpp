/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <list>
#include <unordered_map>
#include <memory>
#include <functional>
#include <SFNUL/Config.hpp>
#include <SFNUL/SyncedObject.hpp>
#include <SFNUL/Link.hpp>
#include <SFNUL/TcpSocket.hpp>

namespace sfn{

class SFNUL_API SynchronizerBase {
public:
	const static Link<ReliableTransport>::stream_id_type stream_id;

protected:
	friend class SyncedObject;

	SynchronizerBase() = default;
	virtual ~SynchronizerBase();

	virtual bool AddObject( SyncedObject* object ) = 0;
	virtual void UpdateObject( SyncedObject* object ) = 0;
	virtual void MoveObject( SyncedObject* object_old, SyncedObject* object_new );
	virtual bool RemoveObject( SyncedObject* object ) = 0;

	bool IsDestroyed() const;

	enum class sync_type : unsigned char {
		create = 100,
		update = 101,
		destroy = 102
	};

	SyncedObject* GetObjectByID( SyncedObject::id_type id );

	std::list<std::weak_ptr<Link<TcpSocket>>> m_links;

	std::list<SyncedObject*> m_objects;

	bool m_destroyed = false;
};

class SFNUL_API SynchronizerServer : public SynchronizerBase {
public:
	/** Ctor.
	 */
	SynchronizerServer();

	/** Dtor.
	 */
	~SynchronizerServer();

	/** Move Ctor.
	 * @param synchronizer SynchronizerServer to move from.
	 */
	SynchronizerServer( SynchronizerServer&& synchronizer );

	/** Move assignment operator.
	 * @param synchronizer SynchronizerServer to move from.
	 */
	SynchronizerServer& operator=( SynchronizerServer&& synchronizer );

	/** Add client Link to this synchronizer.
	 * @param client_link Client Link to add to this synchronizer.
	 * @return bool true if the Link was added.
	 */
	bool AddClient( std::weak_ptr<Link<TcpSocket>> client_link );

	/** Remove client Link from this synchronizer.
	 * @param client_link Client Link to remove from this synchronizer.
	 * @return bool true if the Link was removed.
	 */
	bool RemoveClient( std::weak_ptr<Link<TcpSocket>> client_link );

	/** Update this synchronizer.
	 */
	void Update();

	/** Create an object to be managed by this synchronizer.
	 * @param args Arguments that are forwarded to the object's constructor.
	 * @return New instance of the object.
	 */
	template<typename T, typename... Args>
	T CreateObject( Args&&... args );

protected:
	friend class SyncedObject;

	bool AddObject( SyncedObject* object ) override;
	void UpdateObject( SyncedObject* object ) override;
	bool RemoveObject( SyncedObject* object ) override;

private:
	SynchronizerServer( const SynchronizerServer& synchronizer ) = delete;
	SynchronizerServer& operator=( const SynchronizerServer& synchronizer ) = delete;

	void Send();

	std::unordered_map<SyncedObject::id_type, sync_type> m_updates;
};

class SFNUL_API SynchronizerClient : public SynchronizerBase {
public:
	/** Ctor.
	 */
	SynchronizerClient();

	/** Dtor.
	 */
	~SynchronizerClient();

	/** Move Ctor.
	 * @param synchronizer SynchronizerClient to move from.
	 */
	SynchronizerClient( SynchronizerClient&& synchronizer );

	/** Move assignment operator.
	 * @param synchronizer SynchronizerClient to move from.
	 */
	SynchronizerClient& operator=( SynchronizerClient&& synchronizer );

	/** Add server Link to this synchronizer.
	 * @param server_link Server Link to add to this synchronizer.
	 * @return bool true if the Link was added.
	 */
	bool AddServer( std::weak_ptr<Link<TcpSocket>> server_link );

	/** Remove server Link from this synchronizer.
	 * @param server_link Server Link to remove from this synchronizer.
	 * @return bool true if the Link was removed.
	 */
	bool RemoveServer( std::weak_ptr<Link<TcpSocket>> server_link );

	/** Update this synchronizer.
	 */
	void Update();

	/** Register lifetime managers for objects with the given type id.
	 * @param type_id Object type id to associate the managers with.
	 * @param factory Factory function to create new objects with. Signature: SyncedObject* Factory() Returns a pointer to the newly created object.
	 * @param destructor Destructor function to remove objects with. Signature: void Destructor( SyncedObject* ) Called with a pointer to the object to be removed.
	 */
	void SetLifetimeManagers( SyncedObject::object_type_id_type type_id, std::function<SyncedObject*()> factory, std::function<void( SyncedObject* )> destructor );

protected:
	friend class SyncedObject;

	bool AddObject( SyncedObject* object ) override;
	void UpdateObject( SyncedObject* object ) override;
	bool RemoveObject( SyncedObject* object ) override;

private:
	SynchronizerClient( const SynchronizerClient& synchronizer ) = delete;
	SynchronizerClient& operator=( const SynchronizerClient& synchronizer ) = delete;

	void Receive();

	std::unordered_map<SyncedObject::object_type_id_type, std::function<SyncedObject*()>> m_factories;
	std::unordered_map<SyncedObject::object_type_id_type, std::function<void( SyncedObject* )>> m_destructors;
};

}

#include <SFNUL/Synchronizer.inl>
