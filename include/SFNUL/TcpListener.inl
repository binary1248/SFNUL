/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

namespace sfn {

template<class T>
std::shared_ptr<T> TcpListener::GetPendingConnection() {
	static_assert( std::is_base_of<TcpSocket, T>::value, "TCP Listeners can only accept into TcpSocket or derived classes." );

	auto lock = AcquireLock();

	if( m_new_connections.empty() ) {
		return std::shared_ptr<T>();
	}

	auto socket = T::Create();

	socket->SetInternalSocket( &( m_new_connections.front() ) );

	m_new_connections.pop_front();

	socket->OnConnected();

	socket->m_connected = true;

	socket->ReceiveHandler( asio::error_code{}, 0 );
	socket->SendHandler( asio::error_code{}, 0 );

	return socket;
}

}
