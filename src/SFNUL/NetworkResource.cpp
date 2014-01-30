/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <iostream>
#include <deque>
#include <SFNUL/NetworkResource.hpp>

namespace sfn {

namespace {
	std::deque<std::shared_ptr<Thread>> asio_threads;
	std::shared_ptr<asio::io_service::work> asio_work;
}

std::weak_ptr<asio::io_service> NetworkResource::m_shared_io_service;

NetworkResource::NetworkResource() :
	m_io_service{ m_shared_io_service.expired() ? std::make_shared<asio::io_service>() : m_shared_io_service.lock() },
	m_strand{ *m_io_service }
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
		io_service = std::make_shared<asio::io_service>();
		NetworkResource::m_shared_io_service = io_service;
	}

	asio_work = std::make_shared<asio::io_service::work>( *io_service );

	for( std::size_t index = 0; index < threads; index++ ) {
		asio_threads.emplace_back( std::make_shared<Thread>( [=]() { io_service->run(); } ) );
	}
}

void Stop() {
	auto io_service = NetworkResource::m_shared_io_service.lock();

	if( !io_service ) {
		return;
	}

	asio_work.reset();
	io_service->stop();

	asio_threads.clear();

	io_service->reset();
}

}
