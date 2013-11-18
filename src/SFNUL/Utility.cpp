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

}
