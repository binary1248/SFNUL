/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <type_traits>

namespace sfn {

template<typename T, typename... Args>
T SynchronizerServer::CreateObject( Args&&... args ) {
	static_assert( std::is_base_of<SyncedObject, T>::value, "Cannot call CreateObject on a non-SyncedObject." );

	auto object = T( std::forward<Args>( args )... );

	object.SetSynchronizer( this );

	return object;
}

}
