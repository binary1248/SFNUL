/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <iostream>
#include <sstream>
#include <SFNUL/Config.hpp>

#if defined( ERROR )
#undef ERROR
#endif

namespace sfn {

extern std::ostringstream discard_message;

enum class MessageLevel : unsigned char {
	ERROR = 0,
	WARNING = 1,
	INFORMATION = 2,
	DEBUG = 3,
};

extern MessageLevel message_level;

void SetMessageLevel( MessageLevel level );

std::ostream& ErrorMessage();
std::ostream& WarningMessage();
std::ostream& InformationMessage();
std::ostream& DebugMessage();

}
