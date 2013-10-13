/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <SFNUL/Message.hpp>

namespace sfn {

template<typename T>
SyncedType<T>::SyncedType( SyncedObject* owner ) :
	BaseSyncedType( owner ),
	m_value()
{
}

template<typename T>
SyncedType<T>::SyncedType( SyncedObject* owner, T value ) :
	BaseSyncedType( owner ),
	m_value( value )
{
}

template<typename T>
template<typename S>
void SyncedType<T>::SetValue( T value ) {
	SetModified( true );
	m_value = value;
}

template<typename T>
T SyncedType<T>::GetValue() const {
	return m_value;
}

template<typename T>
template<typename S>
SyncedType<T>& SyncedType<T>::operator=( S other ) {
	SetModified( true );
	m_value = other;
	return *this;
}

#if defined( __GNUG__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#endif

template<typename T>
auto operator++( T& synced_type ) -> decltype( synced_type = ( ++synced_type.GetValue() ) ) {
	return synced_type = ( ++synced_type.GetValue() );
}

template<typename T>
auto operator++( T& synced_type, int ) -> decltype( synced_type = ( synced_type.GetValue()++ ) ) {
	auto return_value = synced_type.GetValue();

	synced_type = ( synced_type.GetValue()++ );

	return return_value;
}

template<typename T>
auto operator--( T& synced_type ) -> decltype( synced_type = ( --synced_type.GetValue() ) ) {
	return synced_type = ( --synced_type.GetValue() );
}

template<typename T>
auto operator--( T& synced_type, int ) -> decltype( synced_type = ( synced_type.GetValue()-- ) ) {
	auto return_value = synced_type.GetValue();

	synced_type = ( synced_type.GetValue()-- );

	return return_value;
}

#if defined( __GNUG__ )
#pragma GCC diagnostic pop
#endif

template<typename T, typename S>
auto operator<<( T synced_type, S other ) -> decltype( synced_type.GetValue() << other ) {
	return synced_type.GetValue() << other;
}

template<typename T, typename S>
auto operator>>( T synced_type, S other ) -> decltype( synced_type.GetValue() >> other ) {
	return synced_type.GetValue() >> other;
}

template<typename T, typename S>
auto operator+=( T& synced_type, S other ) -> decltype( synced_type = synced_type + other ) {
	return synced_type = synced_type + other;
}

template<typename T, typename S>
auto operator-=( T& synced_type, S other ) -> decltype( synced_type = synced_type - other ) {
	return synced_type = synced_type - other;
}

template<typename T, typename S>
auto operator*=( T& synced_type, S other ) -> decltype( synced_type = synced_type * other ) {
	return synced_type = synced_type * other;
}

template<typename T, typename S>
auto operator/=( T& synced_type, S other ) -> decltype( synced_type = synced_type / other ) {
	return synced_type = synced_type / other;
}

template<typename T, typename S>
auto operator%=( T& synced_type, S other ) -> decltype( synced_type = synced_type % other ) {
	return synced_type = synced_type % other;
}

template<typename T, typename S>
auto operator&=( T& synced_type, S other ) -> decltype( synced_type = synced_type & other ) {
	return synced_type = synced_type & other;
}

template<typename T, typename S>
auto operator|=( T& synced_type, S other ) -> decltype( synced_type = synced_type | other ) {
	return synced_type = synced_type | other;
}

template<typename T, typename S>
auto operator^=( T& synced_type, S other ) -> decltype( synced_type = synced_type ^ other ) {
	return synced_type = synced_type ^ other;
}

template<typename T, typename S>
auto operator<<=( T& synced_type, S other ) -> decltype( synced_type = synced_type << other ) {
	return synced_type = synced_type << other;
}

template<typename T, typename S>
auto operator>>=( T& synced_type, S other ) -> decltype( synced_type = synced_type >> other ) {
	return synced_type = synced_type >> other;
}

template<typename T>
template<typename S>
const auto SyncedType<T>::operator[]( S other ) const -> decltype( this->m_value[other] ) {
	return m_value[other];
}

template<typename T>
template<typename S>
auto SyncedType<T>::operator[]( S other ) -> decltype( this->m_value[other] ) {
	SetModified( true );
	return m_value[other];
}

template<typename T>
SyncedType<T>::operator T() const {
	return m_value;
}

template<typename T>
void SyncedType<T>::Serialize( Message& message ) {
	message << m_value;
}

template<typename T>
void SyncedType<T>::Deserialize( Message& message ) {
	message >> m_value;
}

}
