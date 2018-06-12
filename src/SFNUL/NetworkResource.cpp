/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <SFNUL/NetworkResource.hpp>
#include <SFNUL/ConfigInternal.hpp>
#include <SFNUL/MakeUnique.hpp>
#include <asio/io_context.hpp>
#include <asio/strand.hpp>
#include <deque>

namespace {

std::deque<std::shared_ptr<sfn::Thread>> asio_threads;
std::shared_ptr<asio::io_context::work> asio_work;
std::weak_ptr<asio::io_context> shared_io_context;

}

namespace sfn {

class NetworkResource::NetworkResourceImpl {
public:
	NetworkResourceImpl() :
		io_context{ shared_io_context.expired() ? std::make_shared<asio::io_context>() : shared_io_context.lock() },
		strand{ *io_context }
	{
		if( shared_io_context.expired() ) {
			shared_io_context = io_context;
		}
	}

	std::shared_ptr<asio::io_context> io_context;

	mutable asio::io_context::strand strand;
};

NetworkResource::NetworkResource() :
	m_impl{ make_unique<NetworkResourceImpl>() }
{
}

NetworkResource::~NetworkResource() {
}

void* NetworkResource::GetIOService() const {
	return m_impl->io_context.get();
}

void* NetworkResource::GetStrand() const {
	return &m_impl->strand;
}

void Start( std::size_t threads ) {
	auto io_context = shared_io_context.lock();

	if( !io_context ) {
		io_context = std::make_shared<asio::io_context>();
		shared_io_context = io_context;
	}

	asio_work = std::make_shared<asio::io_context::work>( *io_context );

	for( std::size_t index = 0; index < threads; index++ ) {
		asio_threads.emplace_back( std::make_shared<Thread>( [=]() { io_context->run(); } ) );
	}
}

void Stop() {
	auto io_context = shared_io_context.lock();

	if( !io_context ) {
		return;
	}

	asio_work.reset();
	io_context->stop();

	asio_threads.clear();

	io_context->reset();
}

}
