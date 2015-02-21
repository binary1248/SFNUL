/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <SFNUL/Config.hpp>
#include <iosfwd>

namespace sfn {

enum class MessageLevel : unsigned char {
	Error = 0,
	Warning = 1,
	Information = 2,
	Debug = 3,
};

/** Set the severity level of the messages that SFNUL should report about.
 * @param level Severity level.
 */
SFNUL_API void SetMessageLevel( MessageLevel level );

SFNUL_API std::ostream& ErrorMessage();
SFNUL_API std::ostream& WarningMessage();
SFNUL_API std::ostream& InformationMessage();
SFNUL_API std::ostream& DebugMessage();

/** Set the maximum block size you intend to send in a single transfer.
 * @param size Maximum block size.
 */
SFNUL_API void SetMaximumBlockSize( std::size_t size );

/** Get the maximum block size supported in a single transfer.
 * @return Maximum block size supported in a single transfer.
 */
SFNUL_API std::size_t GetMaximumBlockSize();

}
