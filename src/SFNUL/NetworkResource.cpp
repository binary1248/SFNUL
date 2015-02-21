/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <SFNUL/NetworkResource.hpp>
#include <SFNUL/ConfigInternal.hpp>
#include <SFNUL/MakeUnique.hpp>
#include <asio/io_service.hpp>
#include <asio/strand.hpp>
#include <deque>

namespace {

std::deque<std::shared_ptr<sfn::Thread>> asio_threads;
std::shared_ptr<asio::io_service::work> asio_work;
std::weak_ptr<asio::io_service> shared_io_service;

}

namespace sfn {

class NetworkResource::NetworkResourceImpl {
public:
	NetworkResourceImpl() :
		io_service{ shared_io_service.expired() ? std::make_shared<asio::io_service>() : shared_io_service.lock() },
		strand{ *io_service }
	{
		if( shared_io_service.expired() ) {
			shared_io_service = io_service;
		}
	}

	std::shared_ptr<asio::io_service> io_service;

	mutable asio::strand strand;
};

NetworkResource::NetworkResource() :
	m_impl{ make_unique<NetworkResourceImpl>() }
{
}

NetworkResource::~NetworkResource() {
}

void* NetworkResource::GetIOService() const {
	return m_impl->io_service.get();
}

void* NetworkResource::GetStrand() const {
	return &m_impl->strand;
}

void Start( std::size_t threads ) {
	auto io_service = shared_io_service.lock();

	if( !io_service ) {
		io_service = std::make_shared<asio::io_service>();
		shared_io_service = io_service;
	}

	asio_work = std::make_shared<asio::io_service::work>( *io_service );

	for( std::size_t index = 0; index < threads; index++ ) {
		asio_threads.emplace_back( std::make_shared<Thread>( [=]() { io_service->run(); } ) );
	}
}

void Stop() {
	auto io_service = shared_io_service.lock();

	if( !io_service ) {
		return;
	}

	asio_work.reset();
	io_service->stop();

	asio_threads.clear();

	io_service->reset();
}

}
