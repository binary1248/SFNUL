#pragma once

#include <deque>
#include <array>
#include <SFML/System.hpp>
#include <SFNUL/Config.hpp>
#include <SFNUL/NetworkResource.hpp>

namespace sfn {

/** Base socket class.
 */
class SFNUL_API Socket : public NetworkResource {
public:
	/** Constructor.
	 */
	Socket();

	/** Destructor.
	 */
	~Socket();
};

}
