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

enum class MessageLevel : unsigned char {
	ERROR = 0,
	WARNING = 1,
	INFORMATION = 2,
	DEBUG = 3,
};

/** Set the severity level of the messages that SFNUL should report about.
 * @param level Severity level.
 */
void SetMessageLevel( MessageLevel level );

std::ostream& ErrorMessage();
std::ostream& WarningMessage();
std::ostream& InformationMessage();
std::ostream& DebugMessage();

/** Set the maximum block size you intend to send in a single transfer.
 * @param size Maximum block size.
 */
void SetMaximumBlockSize( std::size_t size );

/** Get the maximum block size supported in a single transfer.
 * @return Maximum block size supported in a single transfer.
 */
std::size_t GetMaximumBlockSize();

}
