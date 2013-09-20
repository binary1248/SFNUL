#pragma once

#include <deque>
#include <string>
#include <SFNUL/Config.hpp>
#include <asio/ip/address.hpp>

namespace sfn {

#if defined( __GNUG__ )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#endif

/** An IP address. Can represent IPv4 or IPv6 addresses.
 */
class SFNUL_API IpAddress : public asio::ip::address {

#if defined( __GNUG__ )
#pragma GCC diagnostic pop
#endif

public:
	/** Constructor.
	 */
	IpAddress();

	/// @cond
	IpAddress( const asio::ip::address& asio_address );
	/// @endcond

	/** Constructor.
	 * @param address_string String representation of an IPv4 or IPv6 address.
	 */
	IpAddress( std::string address_string );

	/** Destructor.
	 */
	~IpAddress();

	/** Cast operator to std::string.
	 * Casts this IpAddress into its std::string representation.
	 */
	operator std::string() const;

	/** Check if the address is an IPv4 address.
	 * @return true if the address is an IPv4 address.
	 */
	bool IsIPv4() const;

	/** Check if the address is an IPv6 address.
	 * @return true if the address is an IPv6 address.
	 */
	bool IsIPv6() const;

	/** Resolves a hostname to a std::deque<IpAddress> containing all addresses the hostname identifies.
	 * @param hostname Hostname to resolve.
	 * @return std::deque<IpAddress> containing all addresses the hostname identifies.
	 */
	static std::deque<IpAddress> Resolve( std::string hostname );
};

}
