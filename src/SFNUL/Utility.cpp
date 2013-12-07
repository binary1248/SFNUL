/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <SFNUL/Utility.hpp>

namespace sfn {

std::ostringstream discard_message;

#if defined( SFNUL_DEBUG )
MessageLevel message_level = MessageLevel::DEBUG;
#else
MessageLevel message_level = MessageLevel::ERROR;
#endif

// Maximum bytes that SFNUL will queue before waiting for OS buffer to empty.
// If you think you need support for very large messages, go ahead and change this.
std::size_t block_size = 65536;

void SetMessageLevel( MessageLevel level ) {
	message_level = level;
}

std::ostream& ErrorMessage() {
	if( message_level >= MessageLevel::ERROR ) {
		std::cerr << "SFNUL Error: ";
		return std::cerr;
	}

	discard_message.str( "" );
	return discard_message;
}

std::ostream& WarningMessage() {
	if( message_level >= MessageLevel::WARNING ) {
		std::cerr << "SFNUL Warning: ";
		return std::cerr;
	}

	discard_message.str( "" );
	return discard_message;
}

std::ostream& InformationMessage() {
	if( message_level >= MessageLevel::INFORMATION ) {
		std::cerr << "SFNUL Information: ";
		return std::cerr;
	}

	discard_message.str( "" );
	return discard_message;
}

std::ostream& DebugMessage() {
	if( message_level >= MessageLevel::DEBUG ) {
		std::cerr << "SFNUL Debug: ";
		return std::cerr;
	}

	discard_message.str( "" );
	return discard_message;
}

void SetMaximumBlockSize( std::size_t size ) {
	block_size = size;
}

std::size_t GetMaximumBlockSize() {
	return block_size;
}

}
