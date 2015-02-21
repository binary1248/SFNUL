/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <memory>

#if !defined( _MSC_VER )
// Until C++14 comes along...
template<typename T, typename... Args>
inline std::unique_ptr<T> make_unique( Args&&... args ) {
	return std::unique_ptr<T>( new T( std::forward<Args>( args )... ) );
}
#else
using std::make_unique;
#endif
