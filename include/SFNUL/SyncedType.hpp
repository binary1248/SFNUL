/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <chrono>
#include <SFNUL/Config.hpp>

namespace sfn {

class SyncedObject;
class Message;

enum class SynchronizationType : unsigned char {
	STATIC = 0,
	DYNAMIC = 1,
	STREAM = 2
};

/** Set the period to wait for between synchronizations of STREAM SyncedTypes.
 * @param period Period to wait for.
 */
SFNUL_API void SetStreamSynchronizationPeriod( const std::chrono::milliseconds& period );

class SFNUL_API BaseSyncedType {
public:
	/** Copy assignment operator
	 * @param other BaseSyncedType to assign from.
	 * @return *this
	 */
	BaseSyncedType& operator=( const BaseSyncedType& other );

	/** Move assignment operator
	 * @param other BaseSyncedType to move from.
	 * @return *this
	 */
	BaseSyncedType& operator=( BaseSyncedType&& other );

	/** Check if this object has been modified.
	 * @return true if this object has been modified.
	 */
	bool GetModified() const;

	/** Set whether this object has been modified.
	 * @param modified true to set that this object has been modified, false otherwise.
	 */
	void SetModified( bool modified );

	/** Get the synchronization type set for this object.
	 */
	SynchronizationType GetSynchronizationType() const;

protected:
	friend class SyncedObject;

	friend void SetStreamSynchronizationPeriod( const std::chrono::milliseconds& milliseconds );

	BaseSyncedType( SyncedObject* owner, SynchronizationType sync_type );

	~BaseSyncedType();

	BaseSyncedType( const BaseSyncedType& other ) = delete;

	BaseSyncedType( BaseSyncedType&& other ) = delete;

	virtual void Serialize( Message& message, SynchronizationType sync_type ) = 0;

	virtual void Deserialize( Message& message, SynchronizationType sync_type ) = 0;

	static std::chrono::milliseconds m_sync_period;

private:
	SyncedObject* m_owner = nullptr;

	SynchronizationType m_sync_type;

	bool m_modified = true;
};

template<typename T>
class SFNUL_API SyncedType : BaseSyncedType {
private:
	T m_value;

public:
	explicit SyncedType( SyncedObject* owner, SynchronizationType sync_type = SynchronizationType::DYNAMIC );

	explicit SyncedType( SyncedObject* owner, SynchronizationType sync_type, T value );

	template<typename S>
	void SetValue( T value );

	T GetValue() const;

	template<typename S>
	SyncedType<T>& operator=( S other );

	template<typename S>
	auto operator[]( S other ) const -> decltype( this->m_value[other] );

	template<typename S>
	auto operator[]( S other ) -> decltype( this->m_value[other] );

	operator T() const;

protected:
	virtual void Serialize( Message& message, SynchronizationType sync_type );

	virtual void Deserialize( Message& message, SynchronizationType sync_type );
};

#if defined( __GNUG__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#endif

template<typename T>
auto operator++( T synced_type ) -> decltype( synced_type = ( ++synced_type.GetValue() ) );

template<typename T>
auto operator++( T synced_type, int ) -> decltype( synced_type = ( synced_type.GetValue()++ ) );

template<typename T>
auto operator--( T synced_type ) -> decltype( synced_type = ( --synced_type.GetValue() ) );

template<typename T>
auto operator--( T synced_type, int ) -> decltype( synced_type = ( synced_type.GetValue()-- ) );

#if defined( __GNUG__ )
#pragma GCC diagnostic pop
#endif

template<typename T, typename S>
auto operator<<( T synced_type, S other ) -> decltype( synced_type.GetValue() << other );

template<typename T, typename S>
auto operator>>( T synced_type, S other ) -> decltype( synced_type.GetValue() >> other );

template<typename T, typename S>
auto operator+=( T& synced_type, S other ) -> decltype( synced_type = synced_type + other );

template<typename T, typename S>
auto operator-=( T& synced_type, S other ) -> decltype( synced_type = synced_type - other );

template<typename T, typename S>
auto operator*=( T& synced_type, S other ) -> decltype( synced_type = synced_type * other );

template<typename T, typename S>
auto operator/=( T& synced_type, S other ) -> decltype( synced_type = synced_type / other );

template<typename T, typename S>
auto operator%=( T& synced_type, S other ) -> decltype( synced_type = synced_type % other );

template<typename T, typename S>
auto operator&=( T& synced_type, S other ) -> decltype( synced_type = synced_type & other );

template<typename T, typename S>
auto operator|=( T& synced_type, S other ) -> decltype( synced_type = synced_type | other );

template<typename T, typename S>
auto operator^=( T& synced_type, S other ) -> decltype( synced_type = synced_type ^ other );

template<typename T, typename S>
auto operator<<=( T& synced_type, S other ) -> decltype( synced_type = synced_type << other );

template<typename T, typename S>
auto operator>>=( T& synced_type, S other ) -> decltype( synced_type = synced_type >> other );

}

#include <SFNUL/SyncedType.inl>
