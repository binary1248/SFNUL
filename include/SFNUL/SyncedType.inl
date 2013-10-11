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
auto SyncedType<T>::operator=( S other ) -> decltype( this->m_value = other ) {
	SetModified( true );
	return m_value = other;
}

#if defined( __GNUG__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#endif

template<typename T>
auto SyncedType<T>::operator++() -> decltype( this->m_value++ ) {
	SetModified( true );
	return m_value++;
}

template<typename T>
auto SyncedType<T>::operator++( int ) -> decltype( ++( this->m_value ) ) {
	SetModified( true );
	return ++m_value;
}

template<typename T>
auto SyncedType<T>::operator--() -> decltype( this->m_value-- ) {
	SetModified( true );
	return m_value--;
}

template<typename T>
auto SyncedType<T>::operator--( int ) -> decltype( --( this->m_value ) ) {
	SetModified( true );
	return --m_value;
}

#if defined( __GNUG__ )
#pragma GCC diagnostic pop
#endif

template<typename T>
template<typename S>
auto SyncedType<T>::operator<<( S other ) -> decltype( this->m_value << other ) {
	SetModified( true );
	return m_value << other;
}

template<typename T>
template<typename S>
auto SyncedType<T>::operator>>( S other ) -> decltype( this->m_value >> other ) {
	SetModified( true );
	return m_value >> other;
}

template<typename T>
template<typename S>
auto SyncedType<T>::operator+=( S other ) -> decltype( this->m_value += other ) {
	SetModified( true );
	return m_value += other;
}

template<typename T>
template<typename S>
auto SyncedType<T>::operator-=( S other ) -> decltype( this->m_value -= other ) {
	SetModified( true );
	return m_value -= other;
}

template<typename T>
template<typename S>
auto SyncedType<T>::operator*=( S other ) -> decltype( this->m_value *= other ) {
	SetModified( true );
	return m_value *= other;
}

template<typename T>
template<typename S>
auto SyncedType<T>::operator/=( S other ) -> decltype( this->m_value /= other ) {
	SetModified( true );
	return m_value /= other;
}

template<typename T>
template<typename S>
auto SyncedType<T>::operator%=( S other ) -> decltype( this->m_value %= other ) {
	SetModified( true );
	return m_value %= other;
}

template<typename T>
template<typename S>
auto SyncedType<T>::operator&=( S other ) -> decltype( this->m_value &= other ) {
	SetModified( true );
	return m_value &= other;
}

template<typename T>
template<typename S>
auto SyncedType<T>::operator|=( S other ) -> decltype( this->m_value |= other ) {
	SetModified( true );
	return m_value |= other;
}

template<typename T>
template<typename S>
auto SyncedType<T>::operator^=( S other ) -> decltype( this->m_value ^= other ) {
	SetModified( true );
	return m_value ^= other;
}

template<typename T>
template<typename S>
auto SyncedType<T>::operator<<=( S other ) -> decltype( this->m_value <<= other ) {
	SetModified( true );
	return m_value <<= other;
}

template<typename T>
template<typename S>
auto SyncedType<T>::operator>>=( S other ) -> decltype( this->m_value >>= other ) {
	SetModified( true );
	return m_value >>= other;
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
