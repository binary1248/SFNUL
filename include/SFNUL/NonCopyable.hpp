/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <SFNUL/Config.hpp>

namespace sfn {

/** Class that forbids copying of itself and all derived classes.
 */
class SFNUL_API NonCopyable {
public:
	/// @cond
	NonCopyable() = default;
	NonCopyable( const NonCopyable& ) = delete;
	NonCopyable& operator=( const NonCopyable& ) = delete;

	virtual ~NonCopyable() = default;
	/// @endcond
};

}
