/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <SFNUL/Message.hpp>
#include <ostream>

namespace sfn {

template<typename T>
SyncedType<T>::SyncedType( SyncedObject* owner, SynchronizationType sync_type ) :
	BaseSyncedType( owner, sync_type ),
	m_value()
{
}

template<typename T>
SyncedType<T>::SyncedType( SyncedObject* owner, SynchronizationType sync_type, const T& value ) :
	BaseSyncedType( owner, sync_type ),
	m_value( value )
{
}

template<typename T>
SyncedType<T>::SyncedType( SyncedObject* owner, SynchronizationType sync_type, T&& value ) :
	BaseSyncedType( owner, sync_type ),
	m_value( std::forward<T>( value ) )
{
}

template<typename T>
SyncedType<T>::SyncedType( SyncedObject* owner, const SyncedType<T>& other ) :
	BaseSyncedType( owner, other.m_sync_type ),
	m_value( other.m_value )
{
}

#if defined( __GNUG__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
#endif

template<typename T>
template<typename S>
void SyncedType<T>::SetValue( T value ) {
	if( value != m_value ) {
		SetModified( true );
		m_value = value;
	}
}

template<typename T>
T SyncedType<T>::GetValue() const {
	return m_value;
}

template<typename T>
template<typename S>
SyncedType<T>& SyncedType<T>::operator=( S other ) {
	if( other != m_value ) {
		SetModified( true );
		m_value = other;
	}
	return *this;
}

#if defined( __GNUG__ )
#pragma GCC diagnostic pop
#endif

template<typename T, typename S>
auto operator+( const SyncedType<T>& synced_type, S other ) -> decltype( synced_type.GetValue() + other ) {
	return synced_type.GetValue() + other;
}

template<typename T, typename S>
auto operator+( S other, const SyncedType<T>& synced_type ) -> decltype( other + synced_type.GetValue() ) {
	return other + synced_type.GetValue();
}

template<typename T, typename S>
auto operator-( const SyncedType<T>& synced_type, S other ) -> decltype( synced_type.GetValue() - other ) {
	return synced_type.GetValue() - other;
}

template<typename T, typename S>
auto operator-( S other, const SyncedType<T>& synced_type ) -> decltype( other - synced_type.GetValue() ) {
	return other - synced_type.GetValue();
}

template<typename T, typename S>
auto operator*( const SyncedType<T>& synced_type, S other ) -> decltype( synced_type.GetValue() * other ) {
	return synced_type.GetValue() * other;
}

template<typename T, typename S>
auto operator*( S other, const SyncedType<T>& synced_type ) -> decltype( other * synced_type.GetValue() ) {
	return other * synced_type.GetValue();
}

template<typename T, typename S>
auto operator/( const SyncedType<T>& synced_type, S other ) -> decltype( synced_type.GetValue() / other ) {
	return synced_type.GetValue() / other;
}

template<typename T, typename S>
auto operator/( S other, const SyncedType<T>& synced_type ) -> decltype( other / synced_type.GetValue() ) {
	return other / synced_type.GetValue();
}

template<typename T, typename S>
auto operator%( const SyncedType<T>& synced_type, S other ) -> decltype( synced_type.GetValue() % other ) {
	return synced_type.GetValue() % other;
}

template<typename T, typename S>
auto operator%( S other, const SyncedType<T>& synced_type ) -> decltype( other % synced_type.GetValue() ) {
	return other % synced_type.GetValue();
}

template<typename T, typename S>
auto operator<<( const SyncedType<T>& synced_type, S other ) -> decltype( synced_type.GetValue() << other ) {
	return synced_type.GetValue() << other;
}

template<typename T, typename S>
auto operator<<( S other, const SyncedType<T>& synced_type ) -> decltype( other << synced_type.GetValue() ) {
	return other << synced_type.GetValue();
}

template<typename T, typename S>
auto operator>>( const SyncedType<T>& synced_type, S other ) -> decltype( synced_type.GetValue() >> other ) {
	return synced_type.GetValue() >> other;
}

template<typename T, typename S>
auto operator>>( S other, const SyncedType<T>& synced_type ) -> decltype( other >> synced_type.GetValue() ) {
	return other >> synced_type.GetValue();
}

template<typename T>
auto operator-( const SyncedType<T>& synced_type ) -> decltype( -( synced_type.GetValue() ) ) {
	return -( synced_type.GetValue() );
}

template<typename T, typename S>
auto operator==( const SyncedType<T>& synced_type, S other ) -> decltype( synced_type.GetValue() == other ) {
	return synced_type.GetValue() == other;
}

template<typename T, typename S>
auto operator==( S other, const SyncedType<T>& synced_type ) -> decltype( other == synced_type.GetValue() ) {
	return other == synced_type.GetValue();
}

template<typename T, typename S>
auto operator!=( const SyncedType<T>& synced_type, S other ) -> decltype( synced_type.GetValue() != other ) {
	return synced_type.GetValue() != other;
}

template<typename T, typename S>
auto operator!=( S other, const SyncedType<T>& synced_type ) -> decltype( other != synced_type.GetValue() ) {
	return other != synced_type.GetValue();
}

template<typename T, typename S>
auto operator>( const SyncedType<T>& synced_type, S other ) -> decltype( synced_type.GetValue() > other ) {
	return synced_type.GetValue() > other;
}

template<typename T, typename S>
auto operator>( S other, const SyncedType<T>& synced_type ) -> decltype( other > synced_type.GetValue() ) {
	return other > synced_type.GetValue();
}

template<typename T, typename S>
auto operator<( const SyncedType<T>& synced_type, S other ) -> decltype( synced_type.GetValue() < other ) {
	return synced_type.GetValue() < other;
}

template<typename T, typename S>
auto operator<( S other, const SyncedType<T>& synced_type ) -> decltype( other < synced_type.GetValue() ) {
	return other < synced_type.GetValue();
}

template<typename T, typename S>
auto operator>=( const SyncedType<T>& synced_type, S other ) -> decltype( synced_type.GetValue() >= other ) {
	return synced_type.GetValue() >= other;
}

template<typename T, typename S>
auto operator>=( S other, const SyncedType<T>& synced_type ) -> decltype( other >= synced_type.GetValue() ) {
	return other >= synced_type.GetValue();
}

template<typename T, typename S>
auto operator<=( const SyncedType<T>& synced_type, S other ) -> decltype( synced_type.GetValue() <= other ) {
	return synced_type.GetValue() <= other;
}

template<typename T, typename S>
auto operator<=( S other, const SyncedType<T>& synced_type ) -> decltype( other <= synced_type.GetValue() ) {
	return other <= synced_type.GetValue();
}

template<typename T>
auto operator++( SyncedType<T>& synced_type ) -> decltype( synced_type = ( ++synced_type.GetValue() ) ) {
	return synced_type = ( ++synced_type.GetValue() );
}

template<typename T>
auto operator++( SyncedType<T>& synced_type, int ) -> decltype( synced_type = ( synced_type.GetValue()++ ) ) {
	auto return_value = synced_type.GetValue();

	synced_type = ( synced_type.GetValue()++ );

	return return_value;
}

template<typename T>
auto operator--( SyncedType<T>& synced_type ) -> decltype( synced_type = ( --synced_type.GetValue() ) ) {
	return synced_type = ( --synced_type.GetValue() );
}

template<typename T>
auto operator--( SyncedType<T>& synced_type, int ) -> decltype( synced_type = ( synced_type.GetValue()-- ) ) {
	auto return_value = synced_type.GetValue();

	synced_type = ( synced_type.GetValue()-- );

	return return_value;
}

template<typename T, typename S>
auto operator+=( SyncedType<T>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() + other ) {
	return synced_type = synced_type.GetValue() + other;
}

template<typename T, typename S>
auto operator+=( S& other, const SyncedType<T>& synced_type ) -> decltype( other = other + synced_type.GetValue() ) {
	return other = other + synced_type.GetValue();
}

template<typename T, typename S>
auto operator-=( SyncedType<T>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() - other ) {
	return synced_type = synced_type.GetValue() - other;
}

template<typename T, typename S>
auto operator-=( S& other, const SyncedType<T>& synced_type ) -> decltype( other = other - synced_type.GetValue() ) {
	return other = other - synced_type.GetValue();
}

template<typename T, typename S>
auto operator*=( SyncedType<T>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() * other ) {
	return synced_type = synced_type.GetValue() * other;
}

template<typename T, typename S>
auto operator*=( S& other, const SyncedType<T>& synced_type ) -> decltype( other = other * synced_type.GetValue() ) {
	return other = other * synced_type.GetValue();
}

template<typename T, typename S>
auto operator/=( SyncedType<T>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() / other ) {
	return synced_type = synced_type.GetValue() / other;
}

template<typename T, typename S>
auto operator/=( S& other, const SyncedType<T>& synced_type ) -> decltype( other = other / synced_type.GetValue() ) {
	return other = other / synced_type.GetValue();
}

template<typename T, typename S>
auto operator%=( SyncedType<T>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() % other ) {
	return synced_type = synced_type.GetValue() % other;
}

template<typename T, typename S>
auto operator%=( S& other, const SyncedType<T>& synced_type ) -> decltype( other = other % synced_type.GetValue() ) {
	return other = other % synced_type.GetValue();
}

template<typename T, typename S>
auto operator&=( SyncedType<T>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() & other ) {
	return synced_type = synced_type.GetValue() & other;
}

template<typename T, typename S>
auto operator&=( S& other, const SyncedType<T>& synced_type ) -> decltype( other = other & synced_type.GetValue() ) {
	return other = other & synced_type.GetValue();
}

template<typename T, typename S>
auto operator|=( SyncedType<T>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() | other ) {
	return synced_type = synced_type.GetValue() | other;
}

template<typename T, typename S>
auto operator|=( S& other, const SyncedType<T>& synced_type ) -> decltype( other = other | synced_type.GetValue() ) {
	return other = other | synced_type.GetValue();
}

template<typename T, typename S>
auto operator^=( SyncedType<T>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() ^ other ) {
	return synced_type = synced_type.GetValue() ^ other;
}

template<typename T, typename S>
auto operator^=( S& other, const SyncedType<T>& synced_type ) -> decltype( other = other ^ synced_type.GetValue() ) {
	return other = other ^ synced_type.GetValue();
}

template<typename T, typename S>
auto operator<<=( SyncedType<T>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() << other ) {
	return synced_type = synced_type.GetValue() << other;
}

template<typename T, typename S>
auto operator<<=( S& other, const SyncedType<T>& synced_type ) -> decltype( other = other << synced_type.GetValue() ) {
	return other = other << synced_type.GetValue();
}

template<typename T, typename S>
auto operator>>=( SyncedType<T>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() >> other ) {
	return synced_type = synced_type.GetValue() >> other;
}

template<typename T, typename S>
auto operator>>=( S& other, const SyncedType<T>& synced_type ) -> decltype( other = other >> synced_type.GetValue() ) {
	return other = other >> synced_type.GetValue();
}

template<typename T>
template<typename S>
auto SyncedType<T>::operator[]( S other ) const -> decltype( SyncedType<T>::value_type{}[other] ) {
	return m_value[other];
}

template<typename T>
template<typename S>
auto SyncedType<T>::operator[]( S other ) -> decltype( SyncedType<T>::value_type{}[other] ) {
	SetModified( true );
	return m_value[other];
}

template<typename T>
SyncedType<T>::operator T() const {
	return m_value;
}

template<typename T>
auto SyncedType<T>::operator->() const -> address_type {
	return &( this->m_value );
}

template<typename T>
auto SyncedType<T>::operator->() -> address_type {
	SetModified( true );
	return &( this->m_value );
}

template<typename T>
void SyncedType<T>::Serialize( Message& message, SynchronizationType sync_type ) {
	if( GetSynchronizationType() >= sync_type ) {
		message << m_value;
	}
}

template<typename T>
void SyncedType<T>::Deserialize( Message& message, SynchronizationType sync_type ) {
	if( GetSynchronizationType() >= sync_type ) {
		message >> m_value;
	}
}

template<typename T>
std::ostream& operator<<( std::ostream& stream, const SyncedType<T>& synced_type ) {
	return stream << synced_type.GetValue();
}

}
