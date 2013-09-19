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
