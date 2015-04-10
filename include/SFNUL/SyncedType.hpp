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

/** Get the period to wait for between synchronizations of Stream SyncedTypes.
 * @return Period to wait for.
 */
SFNUL_API std::chrono::milliseconds GetStreamSynchronizationPeriod();

class SFNUL_API BaseSyncedType {
public:
	/** Check if this object has been modified.
	 * @return true if this object has been modified.
	 */
	virtual bool GetModified() const = 0;

	/** Set whether this object has been modified.
	 * @param modified true to set that this object has been modified, false otherwise.
	 */
	virtual void SetModified( bool modified ) = 0;

	/** Get the synchronization type set for this object.
	 * @return The synchronization type set for this object.
	 */
	virtual SynchronizationType GetSynchronizationType() const = 0;

protected:
	friend class SyncedObject;

	BaseSyncedType( SyncedObject* owner, bool stream );

	~BaseSyncedType();

	BaseSyncedType( const BaseSyncedType& other ) = delete;

	BaseSyncedType( BaseSyncedType&& other ) = delete;

	BaseSyncedType& operator=( const BaseSyncedType& other );

	BaseSyncedType& operator=( BaseSyncedType&& other );

	virtual void Serialize( Message& message, SynchronizationType sync_type ) = 0;

	virtual void Deserialize( Message& message, SynchronizationType sync_type ) = 0;

	void NotifyChanged( SyncedObject* owner );
};

template<typename T, SynchronizationType U = SynchronizationType::Dynamic>
class SFNUL_API SyncedType final : public BaseSyncedType {
private:
	SyncedObject* m_owner = nullptr;
	T m_value;
	bool m_modified = true;

public:
	typedef T value_type;

	typedef decltype( &m_value ) address_type;

	/** Constructor.
	 */
	explicit SyncedType( SyncedObject* owner );

	/** Copy constructor (from value_type).
	 */
	explicit SyncedType( SyncedObject* owner, const value_type& value );

	/** Move constructor (from value_type).
	 */
	explicit SyncedType( SyncedObject* owner, value_type&& value );

	/** Copy constructor.
	 */
	template<SynchronizationType V>
	explicit SyncedType( SyncedObject* owner, const SyncedType<value_type, V>& other );

	/** Set the value stored in this SyncedType.
	 * @param value The new value to be stored in this SyncedType.
	 */
	void SetValue( value_type value );

	/** Get the value stored in this SyncedType.
	 * @return The value stored in this SyncedType.
	 */
	value_type GetValue() const;

	/** Assignment operator.
	 */
	template<typename S>
	SyncedType<value_type, U>& operator=( S other );

	/** Array subscript operator.
	 */
	template<typename S>
	auto operator[]( S other ) const -> decltype( value_type{}[other] );

	/** Array subscript operator.
	 */
	template<typename S>
	auto operator[]( S other ) -> decltype( value_type{}[other] );

	/** Type conversion.
	 */
	operator T() const;

	/** Structure dereference operator.
	 */
	auto operator->() const -> address_type;

	/** Structure dereference operator.
	 */
	auto operator->() -> address_type;

private:
	bool GetModified() const override;

	void SetModified( bool modified ) override;

	SynchronizationType GetSynchronizationType() const override;

	void Serialize( Message& message, SynchronizationType sync_type ) override;

	void Deserialize( Message& message, SynchronizationType sync_type ) override;
};

template<typename T, SynchronizationType U, typename S>
auto operator+( const SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type.GetValue() + other );

template<typename T, SynchronizationType U, typename S>
auto operator+( S other, const SyncedType<T, U>& synced_type ) -> decltype( other + synced_type.GetValue() );

template<typename T, SynchronizationType U, typename S>
auto operator-( const SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type.GetValue() - other );

template<typename T, SynchronizationType U, typename S>
auto operator-( S other, const SyncedType<T, U>& synced_type ) -> decltype( other - synced_type.GetValue() );

template<typename T, SynchronizationType U, typename S>
auto operator*( const SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type.GetValue() * other );

template<typename T, SynchronizationType U, typename S>
auto operator*( S other, const SyncedType<T, U>& synced_type ) -> decltype( other * synced_type.GetValue() );

template<typename T, SynchronizationType U, typename S>
auto operator/( const SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type.GetValue() / other );

template<typename T, SynchronizationType U, typename S>
auto operator/( S other, const SyncedType<T, U>& synced_type ) -> decltype( other / synced_type.GetValue() );

template<typename T, SynchronizationType U, typename S>
auto operator%( const SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type.GetValue() % other );

template<typename T, SynchronizationType U, typename S>
auto operator%( S other, const SyncedType<T, U>& synced_type ) -> decltype( other % synced_type.GetValue() );

template<typename T, SynchronizationType U, typename S>
auto operator<<( const SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type.GetValue() << other );

template<typename T, SynchronizationType U, typename S>
auto operator<<( S other, const SyncedType<T, U>& synced_type ) -> decltype( other << synced_type.GetValue() );

template<typename T, SynchronizationType U, typename S>
auto operator>>( const SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type.GetValue() >> other );

template<typename T, SynchronizationType U, typename S>
auto operator>>( S other, const SyncedType<T, U>& synced_type ) -> decltype( other >> synced_type.GetValue() );

template<typename T, SynchronizationType U>
auto operator-( const SyncedType<T, U>& synced_type ) -> decltype( -( synced_type.GetValue() ) );

template<typename T, SynchronizationType U, typename S>
auto operator==( const SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type.GetValue() == other );

template<typename T, SynchronizationType U, typename S>
auto operator==( S other, const SyncedType<T, U>& synced_type ) -> decltype( other == synced_type.GetValue() );

template<typename T, SynchronizationType U, typename S>
auto operator!=( const SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type.GetValue() != other );

template<typename T, SynchronizationType U, typename S>
auto operator!=( S other, const SyncedType<T, U>& synced_type ) -> decltype( other != synced_type.GetValue() );

template<typename T, SynchronizationType U, typename S>
auto operator>( const SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type.GetValue() > other );

template<typename T, SynchronizationType U, typename S>
auto operator>( S other, const SyncedType<T, U>& synced_type ) -> decltype( other > synced_type.GetValue() );

template<typename T, SynchronizationType U, typename S>
auto operator<( const SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type.GetValue() < other );

template<typename T, SynchronizationType U, typename S>
auto operator<( S other, const SyncedType<T, U>& synced_type ) -> decltype( other < synced_type.GetValue() );

template<typename T, SynchronizationType U, typename S>
auto operator>=( const SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type.GetValue() >= other );

template<typename T, SynchronizationType U, typename S>
auto operator>=( S other, const SyncedType<T, U>& synced_type ) -> decltype( other >= synced_type.GetValue() );

template<typename T, SynchronizationType U, typename S>
auto operator<=( const SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type.GetValue() <= other );

template<typename T, SynchronizationType U, typename S>
auto operator<=( S other, const SyncedType<T, U>& synced_type ) -> decltype( other <= synced_type.GetValue() );

template<typename T, SynchronizationType U>
auto operator++( SyncedType<T, U>& synced_type ) -> decltype( synced_type = ( ++( synced_type.GetValue() ) ) );

template<typename T, SynchronizationType U>
auto operator++( SyncedType<T, U>& synced_type, int ) -> decltype( synced_type = ( ( synced_type.GetValue() )++ ) );

template<typename T, SynchronizationType U>
auto operator--( SyncedType<T, U>& synced_type ) -> decltype( synced_type = ( --( synced_type.GetValue() ) ) );

template<typename T, SynchronizationType U>
auto operator--( SyncedType<T, U>& synced_type, int ) -> decltype( synced_type = ( ( synced_type.GetValue() )-- ) );

template<typename T, SynchronizationType U, typename S>
auto operator+=( SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() + other );

template<typename T, SynchronizationType U, typename S>
auto operator+=( S& other, const SyncedType<T, U>& synced_type ) -> decltype( other = other + synced_type.GetValue() );

template<typename T, SynchronizationType U, typename S>
auto operator-=( SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() - other );

template<typename T, SynchronizationType U, typename S>
auto operator-=( S& other, const SyncedType<T, U>& synced_type ) -> decltype( other = other - synced_type.GetValue() );

template<typename T, SynchronizationType U, typename S>
auto operator*=( SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() * other );

template<typename T, SynchronizationType U, typename S>
auto operator*=( S& other, const SyncedType<T, U>& synced_type ) -> decltype( other = other * synced_type.GetValue() );

template<typename T, SynchronizationType U, typename S>
auto operator/=( SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() / other );

template<typename T, SynchronizationType U, typename S>
auto operator/=( S& other, const SyncedType<T, U>& synced_type ) -> decltype( other = other / synced_type.GetValue() );

template<typename T, SynchronizationType U, typename S>
auto operator%=( SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() % other );

template<typename T, SynchronizationType U, typename S>
auto operator%=( S& other, const SyncedType<T, U>& synced_type ) -> decltype( other = other % synced_type.GetValue() );

template<typename T, SynchronizationType U, typename S>
auto operator&=( SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() & other );

template<typename T, SynchronizationType U, typename S>
auto operator&=( S& other, const SyncedType<T, U>& synced_type ) -> decltype( other = other & synced_type.GetValue() );

template<typename T, SynchronizationType U, typename S>
auto operator|=( SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() | other );

template<typename T, SynchronizationType U, typename S>
auto operator|=( S& other, const SyncedType<T, U>& synced_type ) -> decltype( other = other | synced_type.GetValue() );

template<typename T, SynchronizationType U, typename S>
auto operator^=( SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() ^ other );

template<typename T, SynchronizationType U, typename S>
auto operator^=( S& other, const SyncedType<T, U>& synced_type ) -> decltype( other = other ^ synced_type.GetValue() );

template<typename T, SynchronizationType U, typename S>
auto operator<<=( SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() << other );

template<typename T, SynchronizationType U, typename S>
auto operator<<=( S& other, const SyncedType<T, U>& synced_type ) -> decltype( other = other << synced_type.GetValue() );

template<typename T, SynchronizationType U, typename S>
auto operator>>=( SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() >> other );

template<typename T, SynchronizationType U, typename S>
auto operator>>=( S& other, const SyncedType<T, U>& synced_type ) -> decltype( other = other >> synced_type.GetValue() );

template<typename T, SynchronizationType U>
std::ostream& operator<<( std::ostream& stream, const SyncedType<T, U>& synced_type );

template<typename T, SynchronizationType U>
std::istream& operator>>( std::istream& stream, SyncedType<T, U>& synced_type );

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
