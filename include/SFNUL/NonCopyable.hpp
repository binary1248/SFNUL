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
