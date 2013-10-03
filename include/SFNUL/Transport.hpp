/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <SFNUL/Config.hpp>

namespace sfn {

/** Transport interface.
 */
class SFNUL_API Transport {

public:

	/** Destructor.
	 */
	virtual ~Transport();

protected:

	/** Ctor.
	 */
	Transport();

	/** Used to inform subclasses that the transport has sent data.
	 */
	virtual void OnSent();

	/** Used to inform subclasses that the transport has received data.
	 */
	virtual void OnReceived();

	/// @cond
	virtual void SetInternalSocket( void* internal_socket ) = 0;
	/// @endcond

private:

};

}
