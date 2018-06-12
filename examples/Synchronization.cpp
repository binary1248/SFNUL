/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <iostream>
#include <random>
#include <atomic>
#include <tuple>
#include <SFNUL.hpp>

// This is an example of a SyncedObject.
class Object : public sfn::SyncedObject {
public:
	// SFNUL Synchronization part...
	const static object_type_id_type type_id;

	// Requirement #1:
	// You MUST provide at least one non-copy constructor.
	// All Synced member fields MUST be initialized with ( this )
	// or ( this, value ).
	Object() :
		x{ this, 300 },
		y{ this, 200 },
		t{ this, std::make_tuple( 0.0, std::string{} ) }
	{
	}

	// Requirement #2:
	// Copying a synchronized object does not make much sense semantically.
	// This is due to the fact that each SyncedObject has to be unique to enable
	// synchronization across multiple hosts and copying would require complex
	// application specific semantics to be supported.
	// Because 2 SyncedObjects need to be unique, comparing them should result in
	// a == b always being false. This would however lead to strange non-trivial
	// behaviour such as a = b; followed by a == b still being false.
	// Comparisons should therefore take into account the contained values
	// and not the objects themselves.
	// If you still want to enable support for copying, it has to be limited
	// to replicating the contained values. The underlying SyncedObject still
	// has to be created as with the default constructor.
	Object( const Object& object ) :
		x{ this, object.x },
		y{ this, object.y },
		t{ this, object.t }
	{
	}

	Object& operator=( const Object& object ) {
		x = object.x;
		y = object.y;
		t = object.t;

		return *this;
	}

	// Requirement #3:
	// Provide move constructor/assignment operator in the same manner as
	// the above. These are used by standard library containers and functions.
	// Remember to invoke the same operator/constructor of the base SyncedObject
	// class to perform the necessary housekeeping required for synchronization.
	// Be aware that Synced types (SyncedFloat, SyncedUint32, etc.)
	// are neither copyable nor moveable.
	// You must always construct new instances of them when your
	// object is constructed due to the constraints mentioned above.
	Object( Object&& object ) :
		sfn::SyncedObject{ std::forward<sfn::SyncedObject>( object ) },
		x{ this, object.x },
		y{ this, object.y },
		t{ this, object.t }
	{
	}

	Object& operator=( Object&& object ) {
		sfn::SyncedObject::operator=( std::forward<sfn::SyncedObject>( object ) );

		x = object.x;
		y = object.y;
		t = object.t;

		return *this;
	}

	// Requirement #4:
	// In order to recreate new objects on remote systems, the same object type
	// identifiers have to be present on all systems using synchronization.
	// All SyncedObject derived classes have to override this function.
	// We use a static member constant here for demonstration purposes.
	virtual object_type_id_type GetTypeID() const override {
		return type_id;
	}

	// Requirement #5:
	// Synchronized data storage. All types that make use of the
	// SyncedType<T, SyncType> template are automatically synchronized with
	// remote hosts upon mutation.
	// There are provided typedefs for common types such as SyncedInt32,
	// SyncedFloat, SyncedUint16, SyncedBool, etc.
	// It is essential that the ordering of these members stays the same
	// on all systems or they won't get properly synchronized.
	// The synchronization type of the member can be very important depending on
	// what the purpose of the member is and how it interacts with the rest of
	// your application.
	// STATIC synchronization means that the member should only be synchronized
	// at the construction of the object. It is set once and never changed again
	// throughout the lifetime of the object.
	// DYNAMIC synchronization means that the member is synchronized when it is
	// altered. This can be desirable in the case of e.g. user input which has
	// an impact on the value of the member. When the member is not altered,
	// no synchronization will take place.
	// STREAM synchronization means that the member will be synchronized
	// occasionally regardless of whether its value changes or not. This can be
	// desirable e.g. in the case of values that are the result of physical
	// computations or otherwise the derivative of other values that change very
	// often. To prevent STREAM synchronization from consuming a lot of
	// throughput, STREAM members are only synchronized after a user definable
	// period of time has passed since the last synchronization or during a
	// DYNAMIC synchronization. Set the period with
	// SetStreamSynchronizationPeriod() based on testing of the network and
	// state update performance.
	// The default synchronization type is sfn::SynchronizationType::Dynamic.
	// Both of the following types are identical.
	sfn::SyncedType<sfn::Int32, sfn::SynchronizationType::Dynamic> x;
	sfn::SyncedInt32 y;

	// We can manage std::tuples this way as well. However, bear in mind that the
	// entire tuple will be marked as out-of-date as soon as a single element is
	// modified. The tuple will also have a single synchronization type.
	// Use std::get<0>( t.GetValue() ) to get a const reference to an element and
	// std::get<0>( t.Get() ) to get a non-const reference to an element.
	sfn::SyncedType<std::tuple<double, std::string>> t;
};

// Our Object object type id.
const Object::object_type_id_type Object::type_id = 0x1337;

int main( int /*argc*/, char** argv ) {
	std::atomic_bool exit{ false };
	std::cout << "Press ENTER to exit.\n";
	// Don't use sfn::Thread for your own projects, it is not what you think it is.
	sfn::Thread exit_handler( [&]() { std::cin.get(); exit = true; } );

	if( argv[1] && ( argv[1][0] == 's' ) ) {
////////////////////////////////////////////////////////////////////////////////
// Server mode.
////////////////////////////////////////////////////////////////////////////////
		// Create our TCP listener socket.
		auto listener = sfn::TcpListener::Create();

		// Listen on 0.0.0.0:31337
		listener->Listen( sfn::Endpoint{ sfn::IpAddress{ "0.0.0.0" }, 31337 } );

		// A standard STL container to store our objects.
		std::deque<Object> objects{};

		// Our Synchronizer.
		// It is of type SynchronizerServer because in server mode we want it to be
		// the authoritative instance i.e. it holds the master copy of all objects
		// to be synchronized. Any requests by clients to create/destroy/mutate the
		// objects must be forwarded to the server in order to be validated and
		// performed on its copy. This prevents clients from exploiting the
		// game state in their favour by manipulating their copies.
		sfn::SynchronizerServer synchronizer;

		// Start a network processing thread.
		sfn::Start();

		// Synchronizers communicate over Links. They occupy stream id 200.
		// We store them in a simple STL container.
		std::deque<std::shared_ptr<sfn::Link<sfn::TcpSocket>>> links{};

		// A temporary Link to handle incoming connections.
		auto link = std::make_shared<sfn::Link<sfn::TcpSocket>>();

		// All SyncedObject types should be created through a synchronizer as such.
		// The resulting object can be moved around as much as we want, since
		// we took care of supporting moves in our class definition.
		// In this case, the move constructor is invoked.
		objects.emplace_back( synchronizer.CreateObject<Object>() );

		std::mt19937 gen{};
		std::uniform_int_distribution<sfn::Int32> int_dist{ -100, 100 };
		std::uniform_real_distribution<double> real_dist{ -1.0, 1.0 };
		std::uniform_int_distribution<sfn::Uint16> char_dist{ 65, 90 };

		// Move construct a few more new Object objects.
		objects.emplace_back( synchronizer.CreateObject<Object>() );
		objects.emplace_back( synchronizer.CreateObject<Object>() );
		objects.emplace_back( synchronizer.CreateObject<Object>() );
		objects.emplace_back( synchronizer.CreateObject<Object>() );

		for( auto& o : objects ) {
			o.x += int_dist( gen );
			o.y += int_dist( gen );
			std::get<0>( o.t.Get() ) += real_dist( gen );
			std::get<1>( o.t.Get() ) += static_cast<char>( char_dist( gen ) );
			std::get<1>( o.t.Get() ) += static_cast<char>( char_dist( gen ) );
			std::get<1>( o.t.Get() ) += static_cast<char>( char_dist( gen ) );
		}

		while( !exit ) {
			do {
				// Accept all pending connections and bind them to the temporary Link.
				link->SetTransport( listener->GetPendingConnection() );

				// If the Link is connected...
				if( link->GetTransport() && link->GetTransport()->IsConnected() ) {
					// Add it to the synchronizer as a new client.
					synchronizer.AddClient( link );

					// Move it into our housekeeping container and create a new temporary.
					links.emplace_back( std::move( link ) );
					link = std::make_shared<sfn::Link<sfn::TcpSocket>>();
				}
			} while( link->GetTransport() && link->GetTransport()->IsConnected() );

			// Get rid of disconnected Links. The synchronizer will get rid
			// of dead Links automatically. If you want to remove a live Link
			// from a synchronizer or make its removal more explicit, use
			// the RemoveClient() method.
			for( auto iter = std::begin( links ); iter != std::end( links ); ) {
				auto transport = ( *iter )->GetTransport();

				if( !transport ) {
					iter = links.erase( iter );
					continue;
				} else if( !transport->IsConnected() || transport->RemoteHasShutdown() ) {
					transport->Shutdown();
					iter = links.erase( iter );
					continue;
				}
				++iter;
			}

			// Update the synchronizer to broadcast the state to associated hosts.
			synchronizer.Update();

			for( auto& o : objects ) {
				std::cout << "(" << o.x << "," << o.y << "," << std::get<0>( o.t.GetValue() ) << "," << std::get<1>( o.t.GetValue() ) << ") ";
			}
			std::cout << "\n";
		}

		// Gracefully close all connections.
		for( auto& l : links ) {
			if( l && ( l->GetTransport() && l->GetTransport()->IsConnected() ) ) {
				l->GetTransport()->Shutdown();

				while( !l->GetTransport()->RemoteHasShutdown() ) {
				}
			}
		}

		// Stop all network processing threads.
		sfn::Stop();
////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
	} else {
////////////////////////////////////////////////////////////////////////////////
// Client mode.
////////////////////////////////////////////////////////////////////////////////
		// Resolve our hostname to an address.
		auto addresses = sfn::IpAddress::Resolve( "127.0.0.1" );

		// Check if the name resolution was unsuccessful.
		if( addresses.empty() ) {
			std::cout << "Could not resolve hostname \"127.0.0.1\" to an address.\n";
			return 1;
		}

		// Links can be created with an associated transport as well. In this case,
		// you won't need to retrospectively set the transport with SetTransport().
		auto link = std::make_shared<sfn::Link<sfn::TcpSocket>>( sfn::TcpSocket::Create() );

		// Links proxy most methods implemented by their underlying transport.
		// Here we connect to the resolved endpoint.
		link->Connect( sfn::Endpoint{ addresses.front(), 31337 } );

		// A standard STL container to store our objects.
		std::deque<Object> objects{};

		// Our Synchronizer.
		// It is of type SynchronizerClient because in client mode we don't want
		// it to be the authoritative instance i.e. it only receives copies of the
		// master state of all objects to be synchronized. Any requests by the
		// client to create/destroy/mutate the objects must be forwarded to the
		// server in order to be validated and performed on the master copy. This
		// prevents clients from exploiting the game state in their favour by
		// manipulating their local copies of the game state.
		sfn::SynchronizerClient synchronizer;

		// In the case of the client, the synchronizer has to be able to
		// automatically request for objects to be created and destroyed to keep
		// in sync with the master copy. This is why factories and destructors
		// have to be specified for each object type. These are std::function
		// objects and are hence compatible with function pointers, functors,
		// lambda functions/closures, std::bind expressions etc.
		synchronizer.SetLifetimeManagers( Object::type_id,
			// First the factory.
			// This function is called when the synchronizer requires the
			// client to create a new object with the given type id.
			// Simply create the object, store it somewhere, and return
			// a non-owning pointer to it.
			// Signature:
			// sfn::SyncedObject* Factory()
			[&objects]() {
				objects.emplace_back();
				return &objects.back();
			},

			// Then the destructor.
			// This function is called when the synchronizer requires
			// the client to destroy/remove an object from the local
			// state in order to keep in sync with the master copy.
			// The function will be passed a non-owning pointer to the
			// object and the function must remove all instances of
			// the associated object from the local game state.
			// Signature:
			// void Destructor( sfn::SyncedObject* )
			[&objects]( sfn::SyncedObject* object ) {
				auto iter = std::find_if( std::begin( objects ), std::end( objects ),
					[object]( const Object& element ) {
						return object == &element;
					}
				);

				if( iter != std::end( objects ) ) {
					objects.erase( iter );
				}
			}
		);

		// Start a network processing thread.
		sfn::Start();

		// Keeps track of whether we are already connected.
		auto connected = false;

		while( !exit ) {
			// If we aren't already connected and the Link just came alive,
			// add it to the synchronizer as a server and set connected to true.
			if( !connected && link->GetTransport() && link->GetTransport()->IsConnected() ) {
				synchronizer.AddServer( link );
				connected = true;
			}

			// If we are already connected and the Link dies, gracefully
			// shut down and exit the game loop.
			auto transport = link->GetTransport();

			if( connected && ( !transport || !transport->IsConnected() || transport->RemoteHasShutdown() ) ) {
				link->Shutdown();
				break;
			}

			// Update the synchronizer to receive the state from associated hosts.
			synchronizer.Update();

			for( auto& o : objects ) {
				std::cout << "(" << o.x << "," << o.y << "," << std::get<0>( o.t.GetValue() ) << "," << std::get<1>( o.t.GetValue() ) << ") ";
			}
			std::cout << "\n";
		}

		// Gracefully close all connections.
		if( link && ( link->GetTransport() && link->GetTransport()->IsConnected() ) ) {
			link->Shutdown();

			while( !link->RemoteHasShutdown() ) {
			}
		}

		// Stop all network processing threads.
		sfn::Stop();
////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
	}

	return 0;
}
