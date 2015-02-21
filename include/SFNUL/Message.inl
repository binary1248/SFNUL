/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <cstring>
#include <cassert>
#include <type_traits>
#include <array>
#include <iostream>
#include <iterator>

// forward_list support is disabled for the time being...
// Send me a message if you want to convince me that it is useful.
// #define WITH_MESSAGE_FORWARD_LIST_SUPPORT

#if defined( WITH_MESSAGE_FORWARD_LIST_SUPPORT )
#include <forward_list>
#endif

namespace {

// We assume that anyone who is smart enough to implement a class with
// const_iterator support did their homework and implemented the rest of
// the STL container interface as well. SFINAE forever...
template<typename T>
struct has_const_iterator {
private:
	typedef struct { char a[16]; } yes;
	typedef struct { char a[64]; } no; // 64 just to be safe from exotic systems. http://www.gotw.ca/gotw/071.htm

	template<typename C>
	static yes test( typename C::const_iterator* );

	template<typename C>
	static no test( ... );

public:
	static const bool value = sizeof( test<T>( 0 ) ) == sizeof( yes );
};

template<typename T>
struct has_iterator {
private:
	typedef struct { char a[16]; } yes;
	typedef struct { char a[64]; } no;

	template<typename C>
	static yes test( typename C::iterator* );

	template<typename C>
	static no test( ... );

public:
	static const bool value = sizeof( test<T>( 0 ) ) == sizeof( yes );
};

#if defined( SFNUL_ENDIAN_SWAP )

#if defined( _MSC_VER )
#include <intrin.h>
#endif

template<typename T, size_t N>
struct ByteSwapImpl;

template<typename T>
struct ByteSwapImpl<T, 1> {
	void operator()( T& value ) const {
	}
};

template<typename T>
struct ByteSwapImpl<T, 2> {
	void operator()( T& value ) const {
		value = static_cast<T>(
#if defined( _MSC_VER )
			_byteswap_ushort( static_cast<uint16_t>( value ) )
#elif defined( __GNUC__ )
			__builtin_bswap16( static_cast<int16_t>( value ) )
#else
#error No built-in byte-swap supported for this compiler.
#endif
		);
	}
};

template<typename T>
struct ByteSwapImpl<T, 4> {
	void operator()( T& value ) const {
		value = static_cast<T>(
#if defined( _MSC_VER )
			_byteswap_ulong( static_cast<uint32_t>( value ) )
#elif defined( __GNUC__ )
			__builtin_bswap32( static_cast<int32_t>( value ) )
#else
#error No built-in byte-swap supported for this compiler.
#endif
		);
	}
};

template<typename T>
struct ByteSwapImpl<T, 8> {
	void operator()( T& value ) const {
		value = static_cast<T>(
#if defined( _MSC_VER )
			_byteswap_uint64( static_cast<uint64_t>( value ) )
#elif defined( __GNUC__ )
			__builtin_bswap64( static_cast<int64_t>( value ) )
#else
#error No built-in byte-swap supported for this compiler.
#endif
		);
	}
};

template<typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
SFNUL_API void ByteSwap( T& value ) {
	ByteSwapImpl<T, sizeof( T )>()( value );
}

template<typename T, typename std::enable_if<!std::is_integral<T>::value, int>::type = 0>
SFNUL_API void ByteSwap( T& ) {
}

#endif

#if defined( WITH_MESSAGE_FORWARD_LIST_SUPPORT )

template<typename T, typename std::enable_if<std::is_same<T, std::forward_list<typename T::value_type>>::value, int>::type = 0>
SFNUL_API void InsertAtEnd( T& container, typename T::value_type&& value ) {
	auto before_end = container.before_begin();

	for( const auto& e : container ) {
		// Just to make the compiler not warn about unused variable...
		static_cast<decltype( e )>( e );
		++before_end;
	}

	container.insert_after( before_end, std::forward<typename T::value_type>( value ) );
}

template<typename T, typename std::enable_if<!std::is_same<T, std::forward_list<typename T::value_type>>::value, int>::type = 0>
SFNUL_API void InsertAtEnd( T& container, typename T::value_type&& value ) {
	container.insert( std::end( container ), std::forward<typename T::value_type>( value ) );
}

#else

template<typename T>
SFNUL_API void InsertAtEnd( T& container, typename T::value_type&& value ) {
	container.insert( std::end( container ), std::forward<typename T::value_type>( value ) );
}

#endif

}

namespace sfn {

template<typename T, typename std::enable_if<std::is_trivial<T>::value, int>::type = 0>
SFNUL_API Message& operator<<( Message& message, const T& input ) {
	std::vector<char> data( sizeof( T ) );

#if defined( SFNUL_ENDIAN_SWAP )
	T swapped_input( input );

	ByteSwap( swapped_input );

	std::memcpy( data.data(), &swapped_input, sizeof( T ) );
#else
	std::memcpy( data.data(), &input, sizeof( T ) );
#endif

	message.Append( data );

	return message;
}

template<typename T, typename std::enable_if<std::is_trivial<T>::value, int>::type = 0>
SFNUL_API Message& operator>>( const T& input, Message& message ) {
	std::vector<char> data( sizeof( T ) );

#if defined( SFNUL_ENDIAN_SWAP )
	T swapped_input( input );

	ByteSwap( swapped_input );

	std::memcpy( data.data(), &swapped_input, sizeof( T ) );
#else
	std::memcpy( data.data(), &input, sizeof( T ) );
#endif

	message.Prepend( data );

	return message;
}

template<typename T, typename std::enable_if<has_const_iterator<T>::value, int>::type = 0>
SFNUL_API Message& operator<<( Message& message, const T& input ) {
	message << static_cast<Uint32>( input.size() );

	if( std::is_trivial<typename T::value_type>::value ) {
		// Non-recursive implementation.
		std::vector<typename T::value_type> data( input.size() );
		std::copy( std::begin( input ), std::end( input ), std::begin( data ) );
		message.Append( reinterpret_cast<const char*>( data.data() ), input.size() * sizeof( typename T::value_type ) );
	}
	else {
		// Recursive implementation.
		for( const auto& e : input ) {
			message << e;
		}
	}

	return message;
}

/* Dangerous... doesn't do what you expect...
template<typename T, typename std::enable_if<has_const_iterator<T>::value, int>::type = 0>
SFNUL_API Message& operator>>( const T& input, Message& message ) {
	static_cast<Uint32>( input.size() ) >> message;

	if( std::is_trivial<typename T::value_type>::value ) {
		// Non-recursive implementation.
		std::vector<typename T::value_type> data( input.size() );
		std::copy( std::begin( input ), std::end( input ), std::begin( data ) );
		message.Prepend( reinterpret_cast<const char*>( data.data() ), input.size() * sizeof( typename T::value_type ) );
	}
	else {
		// Recursive implementation.
		for( const auto& e : input ) {
			e >> message;
		}
	}

	return message;
}
*/

template<typename T, std::size_t N>
SFNUL_API Message& operator<<( Message& message, const std::array<T, N>& input ) {
	for( const auto& e : input ) {
		message << e;
	}

	return message;
}

template<typename T, std::size_t N, typename std::enable_if<std::is_trivial<T>::value, int>::type = 0>
SFNUL_API Message& operator>>( const std::array<T, N>& input, Message& message ) {
	for( const auto& e : input ) {
		e >> message;
	}

	return message;
}

template<typename T, typename std::enable_if<std::is_trivial<T>::value, int>::type = 0>
SFNUL_API Message& operator>>( Message& message, T& output ) {
	assert( message.GetSize() >= sizeof( T ) );
	std::vector<char> data( message.GetFront( sizeof( T ) ) );

	std::memcpy( &output, data.data(), sizeof( T ) );

#if defined( SFNUL_ENDIAN_SWAP )
	ByteSwap( output );
#endif

	message.PopFront( sizeof( T ) );

	return message;
}

/* Dangerous... doesn't do what you expect...
template<typename T, typename std::enable_if<std::is_trivial<T>::value, int>::type = 0>
SFNUL_API Message& operator<<=( T& output, Message& message ) {
	assert( message.GetSize() >= sizeof( T ) );
	std::vector<char> data( message.GetBack( sizeof( T ) ) );

	std::memcpy( &output, data.data(), sizeof( T ) );

#if defined( SFNUL_ENDIAN_SWAP )
	ByteSwap( output );
#endif

	message.PopBack( sizeof( T ) );

	return message;
}
*/

template<typename T, typename std::enable_if<has_iterator<T>::value, int>::type = 0>
SFNUL_API Message& operator>>( Message& message, T& output ) {
	output.clear();

	Uint32 size;
	message >> size;

	if( std::is_trivial<typename T::value_type>::value ) {
		// Non-recursive implementation.
		std::vector<typename T::value_type> data( size );
		assert( message.GetSize() >= size * sizeof( typename T::value_type ) );
		std::vector<char> char_data( message.GetFront( size * sizeof( typename T::value_type ) ) );
		std::memcpy( data.data(), reinterpret_cast<const char*>( char_data.data() ), size * sizeof( typename T::value_type ) );
		std::copy( std::begin( data ), std::end( data ), std::back_inserter( output ) );
		message.PopFront( size * sizeof( typename T::value_type ) );
	}
	else {
		// Recursive implementation.
		for( Uint32 index = 0; index < size; ++index ) {
			typename T::value_type value;
			message >> value;
			InsertAtEnd( output, std::move( value ) );
		}
	}

	return message;
}

/* Dangerous... doesn't do what you expect
template<typename T, typename std::enable_if<has_iterator<T>::value, int>::type = 0>
SFNUL_API Message& operator<<=( T& output, Message& message ) {
	output.clear();

	Uint32 size;
	size <<= message;

	if( std::is_trivial<typename T::value_type>::value ) {
		// Non-recursive implementation.
		std::vector<typename T::value_type> data( size );
		assert( message.GetSize() >= size * sizeof( typename T::value_type ) );
		std::vector<char> char_data( message.GetBack( size * sizeof( typename T::value_type ) ) );
		std::memcpy( data.data(), reinterpret_cast<const char*>( char_data.data() ), size * sizeof( typename T::value_type ) );
		std::copy( std::begin( data ), std::end( data ), std::back_inserter( output ) );
		message.PopBack( size * sizeof( typename T::value_type ) );
	}
	else {
		// Recursive implementation.
		for( Uint32 index = 0; index < size; ++index ) {
			typename T::value_type value;
			value <<= message;
			InsertAtEnd( output, std::move( value ) );
		}
	}

	return message;
}
*/

template<typename T, std::size_t N>
SFNUL_API Message& operator>>( Message& message, std::array<T, N>& output ) {
	for( std::size_t index = 0; index < N; ++index ) {
		T value;
		message >> value;
		output[index] = std::move( value );
	}

	return message;
}

/* Dangerous... doesn't do what you expect...
template<typename T, std::size_t N>
SFNUL_API Message& operator<<=( std::array<T, N>& output, Message& message ) {
	for( std::size_t index = 0; index < N; ++index ) {
		T value;
		value <<= message;
		output[index] = std::move( value );
	}

	return message;
}
*/

}
