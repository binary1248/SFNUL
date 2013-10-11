/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <SFNUL/Config.hpp>

namespace sfn {

class SyncedObject;
class Message;

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

protected:
	friend class SyncedObject;

	BaseSyncedType( SyncedObject* owner );

	~BaseSyncedType();

	BaseSyncedType( const BaseSyncedType& other ) = delete;

	BaseSyncedType( BaseSyncedType&& other ) = delete;

	virtual void Serialize( Message& message ) = 0;

	virtual void Deserialize( Message& message ) = 0;

private:
	SyncedObject* m_owner = nullptr;

	bool m_modified = true;
};

template<typename T>
class SFNUL_API SyncedType : BaseSyncedType {
private:
	T m_value;

public:
	explicit SyncedType( SyncedObject* owner );

	explicit SyncedType( SyncedObject* owner, T value );

	template<typename S>
	auto operator=( S other ) -> decltype( this->m_value = other );

#if defined( __GNUG__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#endif

	auto operator++() -> decltype( this->m_value++ );

	auto operator++( int ) -> decltype( ++( this->m_value ) );

	auto operator--() -> decltype( this->m_value-- );

	auto operator--( int ) -> decltype( --( this->m_value ) );

#if defined( __GNUG__ )
#pragma GCC diagnostic pop
#endif

	template<typename S>
	auto operator<<( S other ) -> decltype( this->m_value << other );

	template<typename S>
	auto operator>>( S other ) -> decltype( this->m_value >> other );

	template<typename S>
	auto operator+=( S other ) -> decltype( this->m_value += other );

	template<typename S>
	auto operator-=( S other ) -> decltype( this->m_value -= other );

	template<typename S>
	auto operator*=( S other ) -> decltype( this->m_value *= other );

	template<typename S>
	auto operator/=( S other ) -> decltype( this->m_value /= other );

	template<typename S>
	auto operator%=( S other ) -> decltype( this->m_value %= other );

	template<typename S>
	auto operator&=( S other ) -> decltype( this->m_value &= other );

	template<typename S>
	auto operator|=( S other ) -> decltype( this->m_value |= other );

	template<typename S>
	auto operator^=( S other ) -> decltype( this->m_value ^= other );

	template<typename S>
	auto operator<<=( S other ) -> decltype( this->m_value <<= other );

	template<typename S>
	auto operator>>=( S other ) -> decltype( this->m_value >>= other );

	template<typename S>
	const auto operator[]( S other ) const -> decltype( this->m_value[other] );

	template<typename S>
	auto operator[]( S other ) -> decltype( this->m_value[other] );

	operator T() const;

protected:
	virtual void Serialize( Message& message );

	virtual void Deserialize( Message& message );
};

}

#include <SFNUL/SyncedType.inl>
