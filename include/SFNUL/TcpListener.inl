namespace sfn {

template<class T>
std::shared_ptr<T> TcpListener::GetPendingConnection() {
	static_assert( std::is_base_of<TcpSocket, T>::value, "TCP Listeners can only accept into TcpSocket or derived classes." );

	sf::Lock lock{ m_mutex };

	if( m_new_connections.empty() ) {
		return std::shared_ptr<T>();
	}

	auto socket = T::Create();

	socket->SetInternalSocket( &( m_new_connections.front() ) );

	m_new_connections.pop_front();

	socket->m_connected = true;

	socket->ReceiveHandler( asio::error_code{}, 0 );
	socket->SendHandler( asio::error_code{}, 0 );

	return socket;
}

}
