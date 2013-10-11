/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

namespace sfn {

template<typename T>
Link<T>::Link() :
	m_transport{ T::Create() }
{
}

template<typename T>
Link<T>::Link( std::shared_ptr<T> transport ) :
	m_transport{ std::move( transport ) }
{
}

template<typename T>
ReliableTransport* Link<T>::GetInternalTransport() {
	return m_transport.get();
}

template<typename T>
const ReliableTransport* Link<T>::GetInternalTransport() const {
	return m_transport.get();
}

template<typename T>
void Link<T>::SetTransport( std::shared_ptr<T> transport ) {
	m_transport = std::move( transport );
}

template<typename T>
std::shared_ptr<T> Link<T>::GetTransport() {
	return m_transport;
}

template<typename T>
std::shared_ptr<const T> Link<T>::GetTransport() const {
	return m_transport;
}

template<typename T>
Link<T>::operator bool() const {
	return m_transport && m_transport->IsConnected() && !m_transport->RemoteHasShutdown() && !m_transport->LocalHasShutdown();
}

}
