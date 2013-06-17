#include <iostream>
#include <deque>
#include <SFNUL/NetworkResource.hpp>

namespace sfn {

namespace {
	std::deque<std::shared_ptr<sf::Thread>> asio_threads;
	std::shared_ptr<asio::io_service::work> asio_work;
}

std::weak_ptr<asio::io_service> NetworkResource::m_shared_io_service;

NetworkResource::NetworkResource() :
	m_io_service{ m_shared_io_service.expired() ? std::make_shared<asio::io_service>() : m_shared_io_service.lock() },
	m_strand{ *m_io_service },
	m_mutex{}
{
	if( m_shared_io_service.expired() ) {
		m_shared_io_service = m_io_service;
	}
}

NetworkResource::~NetworkResource() {
}

asio::io_service& NetworkResource::GetIOService() const {
	return *m_io_service;
}

void Start( std::size_t threads ) {
	auto io_service = NetworkResource::m_shared_io_service.lock();

	if( !io_service ) {
		return;
	}

	asio_work = std::make_shared<asio::io_service::work>( *io_service );

	for( std::size_t index = 0; index < threads; index++ ) {
		auto thread = std::make_shared<sf::Thread>( [=]() { io_service->run(); } );
		thread->launch();
		asio_threads.push_back( thread );
	}
}

void Stop() {
	auto io_service = NetworkResource::m_shared_io_service.lock();

	if( !io_service ) {
		return;
	}

	asio_work.reset();
	io_service->stop();

	for( auto t : asio_threads ) {
		t->wait();
	}

	io_service->reset();

	asio_threads.clear();
}

}
