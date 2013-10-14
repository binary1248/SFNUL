/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <iostream>
#include <vector>
#include <deque>
#include <array>
#include <list>
#include <string>
#include <SFNUL.hpp>

template<typename T>
void PrintIt( const T& object ) {
	for( const auto& e : object ) {
		PrintIt( e );
	}
}

template<>
void PrintIt( const char& object ) {
	std::cout << object;
}

struct A {
	int a, b, c;
	float d, e, f;
	char g[16];
};

int main() {
	// This is a Message.
	// It is a self-managing container for various kinds of data
	// with stream-like properties. Messages can be used to
	// encapsulate data structures when being sent over a network.
	sfn::Message message;

	int i = 10;
	float f = 3.14f;
	double d = 2.718;

	// Messages support stream-like insertion at its back...
	message << i << f;

	// front { } back << i
	// front { i } back << f
	// front { i, f } back

	// ...and if you were ever annoyed that you couldn't
	// retrospectively add data to the front of such a facility,
	// now you can.
	d >> message;

	// d >> front { i, f } back
	// front { d, i, f } back

	int j = 0;
	float g = 0.f;
	double e = 0;

	// Extraction of data from a Message is done in reverse order of
	// its insertion. Special care has to be taken when inserting data
	// at the front of the Message as it will have to be extracted first.
	message >> e >> j >> g;

	// You could picture the buffer order being reversed
	// front { d, i, f } back
	// back { f, i, d } front >> e
	// back { f, i } front >> j
	// back { f } front >> g
	// back { } front

	std::cout << j << " " << g << " " << e << "\n\n";

	// Messages also support encapsulation of almost all** STL containers.
	// Containers can be nested arbitrarily and the Message should have
	// no problem inserting it.
	// ** std::forward_list is not supported because it doesn't
	// support back insertion efficiently.
	std::vector<std::deque<std::array<std::string, 4>>> local_monster1

	// vector
	{
		// deques
		{
			// arrays
			{
				// strings
				{
					"foo\n",
					"bar\n",
					"baz\n"
				}
			}
		},
		// deques
		{
			// empty
			{
			}
		},
		// deques
		{
			// arrays
			{
				// strings
				{
					"hello\n",
					"world\n"
				}
			}
		},
		// deques
		{
			// arrays
			{
				// empty
				{
				}
			}
		},
		// deques
		{
			// arrays
			{
				// strings
				{
					"C++\n",
					"is\n",
					"awesome\n"
				}
			}
		}
	};

	// Another nested STL data structure.
	std::list<std::string> local_monster2{ "The Second\n" };

	// Messages also support insertion of data structures
	// with trivial layout.
	struct A a1{ 1, 2, 3, 4.f, 5.f, 6.f, "Structure A\n" };

	std::cout << "local_monster1:\n";
	PrintIt( local_monster1 );
	std::cout << "\nlocal_monster2:\n";
	PrintIt( local_monster2 );

	// Unlike plain old data types and trivial data structures,
	// containers carry size information thus they are limited to
	// being inserted at the back of Messages.
	message << local_monster1;

	// front { } back << local_monster1
	// front { local_monster1 } back

	// Trivial data structures can still be inserted at the front.
	a1 >> message;

	// a1 >> front { local_monster1 } back
	// front { a1, local_monster1 } back

	message << a1 << local_monster2;

	// front { a1, local_monster1 } back << a1
	// front { a1, local_monster1, a1 } back << local_monster2
	// front { a1, local_monster1, a1, local_monster2 } back

	// When extracting data from Messages, always make sure the
	// data types match up with the input.
	std::vector<std::deque<std::array<std::string, 4>>> remote_monster1;
	std::list<std::string> remote_monster2;
	struct A a2;

	// Extraction can only be performed from the front of the Message.
	// Remember that the buffer order is reversed during extration.
	message >> a2 >> remote_monster1;

	// front { a1, local_monster1, a1, local_monster2 } back
	// back { local_monster2, a1, local_monster1, a1 } front >> a2
	// back { local_monster2, a1, local_monster1 } front >> remote_monster1
	// back { local_monster2, a1 } front

	message >> a2 >> remote_monster2;

	// back { local_monster2, a1 } front >> a2
	// back { local_monster2 } front >> remote_monster2
	// back { } front

	std::cout << "\n\nremote_monster1:\n";
	PrintIt( remote_monster1 );
	std::cout << "\nremote_monster2:\n";
	PrintIt( remote_monster2 );
	std::cout << "\na2:\n";
	std::cout << a2.a << "\n" << a2.b << "\n" << a2.c << "\n";
	std::cout << a2.d << "\n" << a2.e << "\n" << a2.f << "\n";
	std::cout << a2.g << "\n\n";
}
