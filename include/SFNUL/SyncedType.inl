/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <SFNUL/Message.hpp>
#include <iosfwd>

namespace sfn {

template<typename T, SynchronizationType U>
SyncedType<T, U>::SyncedType( SyncedObject* owner ) :
	BaseSyncedType( owner, U == SynchronizationType::Stream ),
	m_owner( owner ),
	m_value()
{
}

template<typename T, SynchronizationType U>
SyncedType<T, U>::SyncedType( SyncedObject* owner, const T& value ) :
	BaseSyncedType( owner, U == SynchronizationType::Stream ),
	m_owner( owner ),
	m_value( value )
{
}

template<typename T, SynchronizationType U>
SyncedType<T, U>::SyncedType( SyncedObject* owner, T&& value ) :
	BaseSyncedType( owner, U == SynchronizationType::Stream ),
	m_owner( owner ),
	m_value( std::forward<T>( value ) )
{
}

template<typename T, SynchronizationType U>
template<SynchronizationType V>
SyncedType<T, U>::SyncedType( SyncedObject* owner, const SyncedType<T, V>& other ) :
	BaseSyncedType( owner, U == SynchronizationType::Stream ),
	m_owner( owner ),
	m_value( other.m_value )
{
}

template<typename T, SynchronizationType U>
bool SyncedType<T, U>::GetModified() const {
	return m_modified;
}

template<typename T, SynchronizationType U>
void SyncedType<T, U>::SetModified( bool modified ) {
	if( m_modified != modified ) {
		m_modified = modified;

		if( m_modified ) {
			NotifyChanged( m_owner );
		}
	}
}

template<typename T, SynchronizationType U>
SynchronizationType SyncedType<T, U>::GetSynchronizationType() const {
	return U;
}

#if defined( __GNUG__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
#endif

template<typename T, SynchronizationType U>
void SyncedType<T, U>::SetValue( const T& value ) {
	if( value != m_value ) {
		SetModified( true );
		m_value = value;
	}
}

template<typename T, SynchronizationType U>
const T& SyncedType<T, U>::GetValue() const {
	return m_value;
}

template<typename T, SynchronizationType U>
T& SyncedType<T, U>::Get() {
	SetModified( true );
	return m_value;
}

template<typename T, SynchronizationType U>
template<typename S>
SyncedType<T, U>& SyncedType<T, U>::operator=( S other ) {
	if( other != m_value ) {
		SetModified( true );
		m_value = other;
	}
	return *this;
}

#if defined( __GNUG__ )
#pragma GCC diagnostic pop
#endif

template<typename T, SynchronizationType U, typename S>
auto operator+( const SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type.GetValue() + other ) {
	return synced_type.GetValue() + other;
}

template<typename T, SynchronizationType U, typename S>
auto operator+( S other, const SyncedType<T, U>& synced_type ) -> decltype( other + synced_type.GetValue() ) {
	return other + synced_type.GetValue();
}

template<typename T, SynchronizationType U, typename S>
auto operator-( const SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type.GetValue() - other ) {
	return synced_type.GetValue() - other;
}

template<typename T, SynchronizationType U, typename S>
auto operator-( S other, const SyncedType<T, U>& synced_type ) -> decltype( other - synced_type.GetValue() ) {
	return other - synced_type.GetValue();
}

template<typename T, SynchronizationType U, typename S>
auto operator*( const SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type.GetValue() * other ) {
	return synced_type.GetValue() * other;
}

template<typename T, SynchronizationType U, typename S>
auto operator*( S other, const SyncedType<T, U>& synced_type ) -> decltype( other * synced_type.GetValue() ) {
	return other * synced_type.GetValue();
}

template<typename T, SynchronizationType U, typename S>
auto operator/( const SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type.GetValue() / other ) {
	return synced_type.GetValue() / other;
}

template<typename T, SynchronizationType U, typename S>
auto operator/( S other, const SyncedType<T, U>& synced_type ) -> decltype( other / synced_type.GetValue() ) {
	return other / synced_type.GetValue();
}

template<typename T, SynchronizationType U, typename S>
auto operator%( const SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type.GetValue() % other ) {
	return synced_type.GetValue() % other;
}

template<typename T, SynchronizationType U, typename S>
auto operator%( S other, const SyncedType<T, U>& synced_type ) -> decltype( other % synced_type.GetValue() ) {
	return other % synced_type.GetValue();
}

template<typename T, SynchronizationType U, typename S>
auto operator<<( const SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type.GetValue() << other ) {
	return synced_type.GetValue() << other;
}

template<typename T, SynchronizationType U, typename S>
auto operator<<( S other, const SyncedType<T, U>& synced_type ) -> decltype( other << synced_type.GetValue() ) {
	return other << synced_type.GetValue();
}

template<typename T, SynchronizationType U, typename S>
auto operator>>( const SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type.GetValue() >> other ) {
	return synced_type.GetValue() >> other;
}

template<typename T, SynchronizationType U, typename S>
auto operator>>( S other, const SyncedType<T, U>& synced_type ) -> decltype( other >> synced_type.GetValue() ) {
	return other >> synced_type.GetValue();
}

template<typename T, SynchronizationType U>
auto operator-( const SyncedType<T, U>& synced_type ) -> decltype( -( synced_type.GetValue() ) ) {
	return -( synced_type.GetValue() );
}

template<typename T, SynchronizationType U, typename S>
auto operator==( const SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type.GetValue() == other ) {
	return synced_type.GetValue() == other;
}

template<typename T, SynchronizationType U, typename S>
auto operator==( S other, const SyncedType<T, U>& synced_type ) -> decltype( other == synced_type.GetValue() ) {
	return other == synced_type.GetValue();
}

template<typename T, SynchronizationType U, typename S>
auto operator!=( const SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type.GetValue() != other ) {
	return synced_type.GetValue() != other;
}

template<typename T, SynchronizationType U, typename S>
auto operator!=( S other, const SyncedType<T, U>& synced_type ) -> decltype( other != synced_type.GetValue() ) {
	return other != synced_type.GetValue();
}

template<typename T, SynchronizationType U, typename S>
auto operator>( const SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type.GetValue() > other ) {
	return synced_type.GetValue() > other;
}

template<typename T, SynchronizationType U, typename S>
auto operator>( S other, const SyncedType<T, U>& synced_type ) -> decltype( other > synced_type.GetValue() ) {
	return other > synced_type.GetValue();
}

template<typename T, SynchronizationType U, typename S>
auto operator<( const SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type.GetValue() < other ) {
	return synced_type.GetValue() < other;
}

template<typename T, SynchronizationType U, typename S>
auto operator<( S other, const SyncedType<T, U>& synced_type ) -> decltype( other < synced_type.GetValue() ) {
	return other < synced_type.GetValue();
}

template<typename T, SynchronizationType U, typename S>
auto operator>=( const SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type.GetValue() >= other ) {
	return synced_type.GetValue() >= other;
}

template<typename T, SynchronizationType U, typename S>
auto operator>=( S other, const SyncedType<T, U>& synced_type ) -> decltype( other >= synced_type.GetValue() ) {
	return other >= synced_type.GetValue();
}

template<typename T, SynchronizationType U, typename S>
auto operator<=( const SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type.GetValue() <= other ) {
	return synced_type.GetValue() <= other;
}

template<typename T, SynchronizationType U, typename S>
auto operator<=( S other, const SyncedType<T, U>& synced_type ) -> decltype( other <= synced_type.GetValue() ) {
	return other <= synced_type.GetValue();
}

template<typename T, SynchronizationType U>
auto operator++( SyncedType<T, U>& synced_type ) -> decltype( synced_type = ( ++synced_type.GetValue() ) ) {
	return synced_type = ( ++synced_type.GetValue() );
}

template<typename T, SynchronizationType U>
auto operator++( SyncedType<T, U>& synced_type, int ) -> decltype( synced_type = ( synced_type.GetValue()++ ) ) {
	auto return_value = synced_type.GetValue();

	synced_type = ( synced_type.GetValue()++ );

	return return_value;
}

template<typename T, SynchronizationType U>
auto operator--( SyncedType<T, U>& synced_type ) -> decltype( synced_type = ( --synced_type.GetValue() ) ) {
	return synced_type = ( --synced_type.GetValue() );
}

template<typename T, SynchronizationType U>
auto operator--( SyncedType<T, U>& synced_type, int ) -> decltype( synced_type = ( synced_type.GetValue()-- ) ) {
	auto return_value = synced_type.GetValue();

	synced_type = ( synced_type.GetValue()-- );

	return return_value;
}

template<typename T, SynchronizationType U, typename S>
auto operator+=( SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() + other ) {
	return synced_type = synced_type.GetValue() + other;
}

template<typename T, SynchronizationType U, typename S>
auto operator+=( S& other, const SyncedType<T, U>& synced_type ) -> decltype( other = other + synced_type.GetValue() ) {
	return other = other + synced_type.GetValue();
}

template<typename T, SynchronizationType U, typename S>
auto operator-=( SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() - other ) {
	return synced_type = synced_type.GetValue() - other;
}

template<typename T, SynchronizationType U, typename S>
auto operator-=( S& other, const SyncedType<T, U>& synced_type ) -> decltype( other = other - synced_type.GetValue() ) {
	return other = other - synced_type.GetValue();
}

template<typename T, SynchronizationType U, typename S>
auto operator*=( SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() * other ) {
	return synced_type = synced_type.GetValue() * other;
}

template<typename T, SynchronizationType U, typename S>
auto operator*=( S& other, const SyncedType<T, U>& synced_type ) -> decltype( other = other * synced_type.GetValue() ) {
	return other = other * synced_type.GetValue();
}

template<typename T, SynchronizationType U, typename S>
auto operator/=( SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() / other ) {
	return synced_type = synced_type.GetValue() / other;
}

template<typename T, SynchronizationType U, typename S>
auto operator/=( S& other, const SyncedType<T, U>& synced_type ) -> decltype( other = other / synced_type.GetValue() ) {
	return other = other / synced_type.GetValue();
}

template<typename T, SynchronizationType U, typename S>
auto operator%=( SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() % other ) {
	return synced_type = synced_type.GetValue() % other;
}

template<typename T, SynchronizationType U, typename S>
auto operator%=( S& other, const SyncedType<T, U>& synced_type ) -> decltype( other = other % synced_type.GetValue() ) {
	return other = other % synced_type.GetValue();
}

template<typename T, SynchronizationType U, typename S>
auto operator&=( SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() & other ) {
	return synced_type = synced_type.GetValue() & other;
}

template<typename T, SynchronizationType U, typename S>
auto operator&=( S& other, const SyncedType<T, U>& synced_type ) -> decltype( other = other & synced_type.GetValue() ) {
	return other = other & synced_type.GetValue();
}

template<typename T, SynchronizationType U, typename S>
auto operator|=( SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() | other ) {
	return synced_type = synced_type.GetValue() | other;
}

template<typename T, SynchronizationType U, typename S>
auto operator|=( S& other, const SyncedType<T, U>& synced_type ) -> decltype( other = other | synced_type.GetValue() ) {
	return other = other | synced_type.GetValue();
}

template<typename T, SynchronizationType U, typename S>
auto operator^=( SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() ^ other ) {
	return synced_type = synced_type.GetValue() ^ other;
}

template<typename T, SynchronizationType U, typename S>
auto operator^=( S& other, const SyncedType<T, U>& synced_type ) -> decltype( other = other ^ synced_type.GetValue() ) {
	return other = other ^ synced_type.GetValue();
}

template<typename T, SynchronizationType U, typename S>
auto operator<<=( SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() << other ) {
	return synced_type = synced_type.GetValue() << other;
}

template<typename T, SynchronizationType U, typename S>
auto operator<<=( S& other, const SyncedType<T, U>& synced_type ) -> decltype( other = other << synced_type.GetValue() ) {
	return other = other << synced_type.GetValue();
}

template<typename T, SynchronizationType U, typename S>
auto operator>>=( SyncedType<T, U>& synced_type, S other ) -> decltype( synced_type = synced_type.GetValue() >> other ) {
	return synced_type = synced_type.GetValue() >> other;
}

template<typename T, SynchronizationType U, typename S>
auto operator>>=( S& other, const SyncedType<T, U>& synced_type ) -> decltype( other = other >> synced_type.GetValue() ) {
	return other = other >> synced_type.GetValue();
}

template<typename T, SynchronizationType U>
template<typename S>
auto SyncedType<T, U>::operator[]( S other ) const -> decltype( SyncedType<T, U>::value_type{}[other] ) {
	return m_value[other];
}

template<typename T, SynchronizationType U>
template<typename S>
auto SyncedType<T, U>::operator[]( S other ) -> decltype( SyncedType<T, U>::value_type{}[other] ) {
	SetModified( true );
	return m_value[other];
}

template<typename T, SynchronizationType U>
SyncedType<T, U>::operator const T&() const {
	return m_value;
}

template<typename T, SynchronizationType U>
SyncedType<T, U>::operator T&() {
	SetModified( true );
	return m_value;
}

template<typename T, SynchronizationType U>
auto SyncedType<T, U>::operator->() const -> address_type {
	return &( this->m_value );
}

template<typename T, SynchronizationType U>
auto SyncedType<T, U>::operator->() -> address_type {
	SetModified( true );
	return &( this->m_value );
}

template<typename T, SynchronizationType U>
void SyncedType<T, U>::Serialize( Message& message, SynchronizationType sync_type ) {
	if( GetSynchronizationType() >= sync_type ) {
		message << m_value;
	}
}

template<typename T, SynchronizationType U>
void SyncedType<T, U>::Deserialize( Message& message, SynchronizationType sync_type ) {
	if( GetSynchronizationType() >= sync_type ) {
		message >> m_value;
	}
}

template<typename T, SynchronizationType U>
std::ostream& operator<<( std::ostream& stream, const SyncedType<T, U>& synced_type ) {
	return stream << synced_type.GetValue();
}

template<typename T, SynchronizationType U>
std::istream& operator>>( std::istream& stream, SyncedType<T, U>& synced_type ) {
	T value;
	auto& result = stream >> value;
	synced_type.SetValue( value );
	return result;
}

}
