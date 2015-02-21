/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <SFNUL/ConfigInternal.hpp>
#include <asio/ip/address.hpp>

namespace sfn {

class IpAddress::IpAddressImpl {
public:
	asio::ip::address address;
};

}
