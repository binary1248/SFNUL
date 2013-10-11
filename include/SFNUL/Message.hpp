/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <deque>
#include <vector>
#include <SFNUL/Config.hpp>

namespace sfn {

/** A simple message class to transport ordered data in
 */
class SFNUL_API Message {
public:
	typedef Uint32 size_type;

	/** Clear the message of its contents.
	 */
	void Clear();

	/** Prepend data to the message.
	 * @param data Pointer to a block of memory containing the data to prepend.
	 * @param size Size of the block of memory containing the data to prepend.
	 */
	void Prepend( const char* data, std::size_t size );

	/** Prepend data to the message.
	 * @param data std::vector<char> containing the data to prepend.
	 */
	void Prepend( const std::vector<char>& data );

	/** Append data to the message.
	 * @param data Pointer to a block of memory containing the data to append.
	 * @param size Size of the block of memory containing the data to append.
	 */
	void Append( const char* data, std::size_t size );

	/** Append data to the message.
	 * @param data std::vector<char> containing the data to append.
	 */
	void Append( const std::vector<char>& data );

	/** Get the size of the data contained in this message.
	 * @return Size of the data contained in this message in bytes.
	 */
	size_type GetSize() const;

	/** Get the first n elements in this message.
	 * @param size Number of elements to get.
	 * @return std::vector<char> containing the first n elements in this message.
	 */
	std::vector<char> GetFront( std::size_t size = 1 );

	/** Get the last n elements in this message.
	 * @param size Number of elements to get.
	 * @return std::vector<char> containing the last n elements in this message.
	 */
	std::vector<char> GetBack( std::size_t size = 1 );

	/** Remove the first n elements in this message.
	 * @param size Number of elements to remove.
	 */
	void PopFront( std::size_t size = 1 );

	/** Remove the last n elements in this message.
	 * @param size Number of elements to remove.
	 */
	void PopBack( std::size_t size = 1 );

	/** Get the internal buffer associated with this Message object.
	 * @return Internal buffer associated with this Message object.
	 */
	const std::deque<unsigned char>& GetBuffer() const;

private:
	std::deque<unsigned char> m_data{};
};

}

#include <SFNUL/Message.inl>
