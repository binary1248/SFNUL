/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <SFNUL/Config.hpp>
#include <SFNUL/IpAddress.hpp>
#include <SFNUL/Endpoint.hpp>
#include <SFNUL/TcpSocket.hpp>
#include <SFNUL/UdpSocket.hpp>
#include <SFNUL/TcpListener.hpp>
#include <SFNUL/TlsConnection.hpp>
#include <SFNUL/HTTP.hpp>
#include <SFNUL/HTTPClient.hpp>
#include <SFNUL/SyncedObject.hpp>
#include <SFNUL/SyncedType.hpp>
#include <SFNUL/Message.hpp>
#include <SFNUL/Link.hpp>
#include <SFNUL/Synchronizer.hpp>
#include <SFNUL/Utility.hpp>

namespace sfn {

typedef SyncedType<Uint8> SyncedBool;

typedef SyncedType<Int8> SyncedInt8;
typedef SyncedType<Uint8> SyncedUint8;

typedef SyncedType<Int16> SyncedInt16;
typedef SyncedType<Uint16> SyncedUint16;

typedef SyncedType<Int32> SyncedInt32;
typedef SyncedType<Uint32> SyncedUint32;

typedef SyncedType<Int64> SyncedInt64;
typedef SyncedType<Uint64> SyncedUint64;

typedef SyncedType<float> SyncedFloat;
typedef SyncedType<double> SyncedDouble;

}
