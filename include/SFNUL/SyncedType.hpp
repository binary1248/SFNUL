/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <SFNUL/Config.hpp>
#include <SFNUL/DataTypes.hpp>
#include <chrono>
#include <iosfwd>

namespace sfn {

class SyncedObject;
class Message;

enum class SynchronizationType : unsigned char {
	Static = 0,
	Dynamic = 1,
	Stream = 2
};

/** Set the period to wait for between synchronizations of Stream SyncedTypes.
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

	friend void SetStreamSynchronizationPeriod( const std::chrono::milliseconds& period );

	BaseSyncedType( SyncedObject* owner, SynchronizationType sync_type );

	virtual ~BaseSyncedType();

	BaseSyncedType( const BaseSyncedType& other ) = delete;

	BaseSyncedType( BaseSyncedType&& other ) = delete;

	virtual void Serialize( Message& message, SynchronizationType sync_type ) = 0;

	virtual void Deserialize( Message& message, SynchronizationType sync_type ) = 0;

	static std::chrono::milliseconds m_sync_period;

	SynchronizationType m_sync_type;

private:
	SyncedObject* m_owner = nullptr;

	bool m_modified = true;
};

template<typename T>
class SFNUL_API SyncedType : BaseSyncedType {
public:
	typedef T value_type;

private:
	value_type m_value;

	typedef decltype( &m_value ) address_type;

public:
	explicit SyncedType( SyncedObject* owner, SynchronizationType sync_type = SynchronizationType::Dynamic );

	explicit SyncedType( SyncedObject* owner, SynchronizationType sync_type, const value_type& value );

	explicit SyncedType( SyncedObject* owner, SynchronizationType sync_type, value_type&& value );

	explicit SyncedType( SyncedObject* owner, const SyncedType<value_type>& other );

	template<typename S>
	void SetValue( value_type value );

	value_type GetValue() const;

	template<typename S>
	SyncedType<value_type>& operator=( S other );

	template<typename S>
	auto operator[]( S other ) const -> decltype( value_type{}[other] );

	template<typename S>
	auto operator[]( S other ) -> decltype( value_type{}[other] );

	operator T() const;

	auto operator->() const -> address_type;

	auto operator->() -> address_type;

protected:
	void Serialize( Message& message, SynchronizationType sync_type ) override;

	void Deserialize( Message& message, SynchronizationType sync_type ) override;
};

template<typename T, typename S>
auto operator+( const SyncedType<T>& synced_type, S other ) -> decltype( synced_type.GetValue() + other );

template<typename T, typename S>
auto operator+( S other, const SyncedType<T>& synced_type ) -> decltype( other + synced_type.GetValue() );

template<typename T, typename S>
auto operator-( const SyncedType<T>& synced_type, S other ) -> decltype( synced_type.GetValue() - other );

template<typename T, typename S>
auto operator-( S other, const SyncedType<T>& synced_type ) -> decltype( other - synced_type.GetValue() );

template<typename T, typename S>
auto operator*( const SyncedType<T>& synced_type, S other ) -> decltype( synced_type.GetValue() * other );

template<typename T, typename S>
auto operator*( S other, const SyncedType<T>& synced_type ) -> decltype( other * synced_type.GetValue() );

template<typename T, typename S>
auto operator/( const SyncedType<T>& synced_type, S other ) -> decltype( synced_type.GetValue() / other );

template<typename T, typename S>
auto operator/( S other, const SyncedType<T>& synced_type ) -> decltype( other / synced_type.GetValue() );

template<typename T, typename S>
auto operator%( const SyncedType<T>& synced_type, S other ) -> decltype( synced_type.GetValue() % other );

template<typename T, typename S>
auto operator%( S other, const SyncedType<T>& synced_type ) -> decltype( other % synced_type.GetValue() );

template<typename T, typename S>
auto operator<<( const SyncedType<T>& synced_type, S other ) -> decltype( synced_type.GetValue() << other );

template<typename T, typename S>
auto operator<<( S other, const SyncedType<T>& synced_type ) -> decltype( other << synced_type.GetValue() );

template<typename T, typename S>
auto operator>>( const SyncedType<T>& synced_type, S other ) -> decltype( synced_type.GetValue() >> other );

template<typename T, typename S>
auto operator>>( S other, const SyncedType<T>& synced_type ) -> decltype( other >> synced_type.GetValue() );

template<typename T>
auto operator-( const SyncedType<T>& synced_type ) -> decltype( -( synced_type.GetValue() ) );

template<typename T, typename S>
auto operator==( const SyncedType<T>& synced_type, S other ) -> decltype( synced_type.GetValue() == other );

template<typename T, typename S>
auto operator==( S other, const SyncedType<T>& synced_type ) -> decltype( other == synced_type.GetValue() );

template<typename T, typename S>
auto operator!=( const SyncedType<T>& synced_type, S other ) -> decltype( synced_type.GetValue() != other );

template<typename T, typename S>
auto operator!=( S other, const SyncedType<T>& synced_type ) -> decltype( other != synced_type.GetValue() );

template<typename T, typename S>
auto operator>( const SyncedType<T>& synced_type, S other ) -> decltype( synced_type.GetValue() > other );

template<typename T, typename S>
auto operator>( S other, const SyncedType<T>& synced_type ) -> decltype( other > synced_type.GetValue() );

template<typename T, typename S>
auto operator<( const SyncedType<T>& synced_type, S other ) -> decltype( synced_type.GetValue() < other );

template<typename T, typename S>
auto operator<( S other, const SyncedType<T>& synced_type ) -> decltype( other < synced_type.GetValue() );

template<typename T, typename S>
auto operator>=( const SyncedType<T>& synced_type, S other ) -> decltype( synced_type.GetValue() >= other );

template<typename T, typename S>
auto operator>=( S other, const SyncedType<T>& synced_type ) -> decltype( other >= synced_type.GetValue() );

template<typename T, typename S>
auto operator<=( const SyncedType<T>& synced_type, S other ) -> decltype( synced_type.GetValue() <= other );

template<typename T, typename S>
auto operator<=( S other, const SyncedType<T>& synced_type ) -> decltype( other <= synced_type.GetValue() );

template<typename T>
auto operator++( SyncedType<T>& synced_type ) -> decltype( synced_type = ( ++( synced_type.GetValue() ) ) );

template<typename T>
auto operator++( SyncedType<T>& synced_type, int ) -> decltype( synced_type = ( ( synced_type.GetValue() )++ ) );

template<typename T>
auto operator--( SyncedType<T>& synced_type ) -> decltype( synced_type = ( --( synced_type.GetValue() ) ) );

template<typename T>
auto operator--( SyncedType<T>& synced_type, int ) -> decltype( synced_type = ( ( synced_type.GetValue() )-- ) );

template<typename T, typename S>
auto operator+=( SyncedType<T>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() + other );

template<typename T, typename S>
auto operator+=( S& other, const SyncedType<T>& synced_type ) -> decltype( other = other + synced_type.GetValue() );

template<typename T, typename S>
auto operator-=( SyncedType<T>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() - other );

template<typename T, typename S>
auto operator-=( S& other, const SyncedType<T>& synced_type ) -> decltype( other = other - synced_type.GetValue() );

template<typename T, typename S>
auto operator*=( SyncedType<T>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() * other );

template<typename T, typename S>
auto operator*=( S& other, const SyncedType<T>& synced_type ) -> decltype( other = other * synced_type.GetValue() );

template<typename T, typename S>
auto operator/=( SyncedType<T>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() / other );

template<typename T, typename S>
auto operator/=( S& other, const SyncedType<T>& synced_type ) -> decltype( other = other / synced_type.GetValue() );

template<typename T, typename S>
auto operator%=( SyncedType<T>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() % other );

template<typename T, typename S>
auto operator%=( S& other, const SyncedType<T>& synced_type ) -> decltype( other = other % synced_type.GetValue() );

template<typename T, typename S>
auto operator&=( SyncedType<T>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() & other );

template<typename T, typename S>
auto operator&=( S& other, const SyncedType<T>& synced_type ) -> decltype( other = other & synced_type.GetValue() );

template<typename T, typename S>
auto operator|=( SyncedType<T>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() | other );

template<typename T, typename S>
auto operator|=( S& other, const SyncedType<T>& synced_type ) -> decltype( other = other | synced_type.GetValue() );

template<typename T, typename S>
auto operator^=( SyncedType<T>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() ^ other );

template<typename T, typename S>
auto operator^=( S& other, const SyncedType<T>& synced_type ) -> decltype( other = other ^ synced_type.GetValue() );

template<typename T, typename S>
auto operator<<=( SyncedType<T>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() << other );

template<typename T, typename S>
auto operator<<=( S& other, const SyncedType<T>& synced_type ) -> decltype( other = other << synced_type.GetValue() );

template<typename T, typename S>
auto operator>>=( SyncedType<T>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() >> other );

template<typename T, typename S>
auto operator>>=( S& other, const SyncedType<T>& synced_type ) -> decltype( other = other >> synced_type.GetValue() );

template<typename T>
std::ostream& operator<<( std::ostream& stream, const SyncedType<T>& synced_type );

typedef SyncedType<Uint8> SyncedBool;

typedef SyncedType<Int8> SyncedInt8;
typedef SyncedType<Uint8> SyncedUint8;

typedef SyncedType<Int16> SyncedInt16;
typedef SyncedType<Uint16> SyncedUint16;

typedef SyncedType<Int32> SyncedInt32;
typedef SyncedType<Uint32> SyncedUint32;

typedef SyncedType<Int64> SyncedInt64;
typedef SyncedType<Uint64> SyncedUint64;

typedef SyncedType<float> SyncedFloat;
typedef SyncedType<double> SyncedDouble;

}

#include <SFNUL/SyncedType.inl>
