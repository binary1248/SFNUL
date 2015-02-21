/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <SFNUL/Config.hpp>

namespace sfn {

class Endpoint;

/** Transport interface.
 */
class SFNUL_API Transport {

public:

	/** Destructor.
	 */
	virtual ~Transport();

	/** Get the local endpoint of the established connection this transport is part of.
	 * @return Local endpoint of the established connection this transport is part of.
	 */
	virtual Endpoint GetLocalEndpoint() const = 0;

	/** Clear the send and receive queues of this transport.
	 */
	virtual void ClearBuffers() = 0;

protected:

	/** Constructor.
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
