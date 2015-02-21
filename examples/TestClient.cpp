/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <cmath>
#include <iostream>
#include <list>
#include <random>
#include <atomic>
#include <SFML/Graphics.hpp>
#include <SFNUL.hpp>

namespace sfn {

// Operator overloads for our convenience.
sfn::Message& operator<<( sfn::Message& message, const sf::Color& input );
sfn::Message& operator>>( sfn::Message& message, sf::Color& output );
sfn::Message& operator<<( sfn::Message& message, const sf::Vector2f& input );
sfn::Message& operator>>( sfn::Message& message, sf::Vector2f& output );
sfn::Message& operator<<( sfn::Message& message, const sf::String& input );
sfn::Message& operator>>( sfn::Message& message, sf::String& output );

}

// Our IDs for the different streams.
enum StreamID : sfn::LinkBase::stream_id_type {
	Default = 0, // You shouldn't touch this unless you don't use the default stream anyway.
	PlayerCommandStream = 8,
	PlayerChatStream,
	FontDataStream,
	SyncStream = 200 // DO NOT TOUCH STREAM IDS >= 200
};

// Our player commands.
enum PlayerCommand : sfn::Uint8 {
	LeftStart,
	LeftStop,
	RightStart,
	RightStop,
	Accelerate,
	Decelerate,
	StartFireWeapon,
	StopFireWeapon
};

// Simple synchronized chat log.
class ChatLog : public sfn::SyncedObject {
public:
	const static object_type_id_type type_id;

	ChatLog() :
		m_chat_messages{ this, sfn::SynchronizationType::Dynamic }
	{
	}

	ChatLog( ChatLog&& chatlog ) :
		sfn::SyncedObject{ std::forward<sfn::SyncedObject>( chatlog ) },
		m_chat_messages{ this, chatlog.m_chat_messages }
	{
	}

	void TryLoadFont() {
		if( m_font_data.empty() ) {
			return;
		}

		m_font.loadFromMemory( m_font_data.c_str(), m_font_data.size() );
		m_font_loaded = true;
	}

	void Draw( sf::RenderWindow& window ) {
		if( !m_font_loaded ) {
			TryLoadFont();
		}

		for( std::size_t i = 0; i < m_chat_messages->size(); ++i ) {
			sf::Text text{ m_chat_messages[i], m_font };
			text.setCharacterSize( 14 );
			text.setStyle( sf::Text::Bold );
			text.setColor( sf::Color{ 200, 200, 200, static_cast<sf::Uint8>( 200 - i * 30 ) } );
			text.setPosition( { 4.f, 350.f - static_cast<float>( i ) * 15.f } );

			window.draw( text );
		}

		sf::Text text{ "Accelerate: W\nLeft: A\nRight: D\nFire: Space\nChat: Enter", m_font };
		text.setCharacterSize( 14 );
		text.setStyle( sf::Text::Bold );
		text.setColor( sf::Color{ 200, 200, 200, 127 } );
		text.setPosition( { 470.f, 5.f } );

		window.draw( text );
	}

	void DrawCurrentMessage( sf::RenderWindow& window, const sf::String& message, bool has_name = true ) {
		if( !m_font_loaded ) {
			TryLoadFont();
		}

		sf::Text text{ ( has_name ? sf::String{ "Say: " } : sf::String{ "Your name is: " } ) + message, m_font };
		text.setCharacterSize( 14 );
		text.setStyle( sf::Text::Bold );
		text.setColor( sf::Color::White );
		text.setPosition( { 4.f, 370.f } );

		window.draw( text );
	}

	static void SetFontData( std::string data ) {
		m_font_data = std::move( data );
	}

protected:
	virtual object_type_id_type GetTypeID() const override {
		return type_id;
	}

private:
	sfn::SyncedType<std::deque<sf::String>> m_chat_messages;

	static std::string m_font_data;

	sf::Font m_font{};

	bool m_font_loaded{ false };
};

const ChatLog::object_type_id_type ChatLog::type_id = 0x1338;
std::string ChatLog::m_font_data{};

// I was going to call this Projectile, but I figured I should pay homage to all those threads...
class Bullet : public sfn::SyncedObject {
public:
	const static object_type_id_type type_id;

	Bullet() :
		m_position{ this, sfn::SynchronizationType::Stream },
		m_velocity{ this, sfn::SynchronizationType::Dynamic }
	{
		m_shape.setRadius( 5 );
		m_shape.setOrigin( { 5.f, 5.f } );
		m_shape.setOutlineThickness( 0 );
	}

	Bullet( Bullet&& bullet ) :
		sfn::SyncedObject{ std::forward<sfn::SyncedObject>( bullet ) },
		m_position{ this, bullet.m_position },
		m_velocity{ this, bullet.m_velocity }
	{
	}

	void Update() {
		// Our super complicated physics engine. Copy & Pasted from server.
		auto delta = m_update_clock.restart().asSeconds();

		m_position += m_velocity * delta;

		m_shape.setPosition( m_position );
		auto new_color = m_shape.getFillColor();

		const static auto multiplier = 4.f;
		new_color.r = static_cast<sf::Uint8>( 200 + ( new_color.r + static_cast<int>( delta * 251.f * multiplier ) ) % 60 );
		new_color.g = static_cast<sf::Uint8>( 200 + ( new_color.g - static_cast<int>( delta * 199.f * multiplier ) ) % 60 );
		new_color.b = static_cast<sf::Uint8>( 200 + ( new_color.b + static_cast<int>( delta * 163.f * multiplier ) ) % 60 );
		m_shape.setFillColor( new_color );

		m_shape.setPointCount( 2 + ( m_shape.getPointCount() + 3 ) % 8 );
	}

	void Draw( sf::RenderWindow& window ) const {
		window.draw( m_shape );
	}

protected:
	virtual object_type_id_type GetTypeID() const override {
		return type_id;
	}

private:
	sfn::SyncedType<sf::Vector2f> m_position;
	sfn::SyncedType<sf::Vector2f> m_velocity;

	sf::Clock m_update_clock{};

	sf::CircleShape m_shape{};
};

const Bullet::object_type_id_type Bullet::type_id = 0x1339;

// Player class representing a client-controllable entity.
class Player : public sfn::SyncedObject {
public:
	const static object_type_id_type type_id;

	Player() :
		m_position{ this, sfn::SynchronizationType::Stream },
		m_velocity{ this, sfn::SynchronizationType::Stream },
		m_acceleration{ this, sfn::SynchronizationType::Dynamic },
		m_rotation{ this, sfn::SynchronizationType::Stream },
		m_rotational_velocity{ this, sfn::SynchronizationType::Dynamic },
		m_color{ this, sfn::SynchronizationType::Static }
	{
		m_shape.setPoint( 0, { 0.f, -15.f } );
		m_shape.setPoint( 1, { -10.f, 15.f } );
		m_shape.setPoint( 2, { 10.f, 15.f } );
	}

	Player( Player&& player ) :
		sfn::SyncedObject{ std::forward<sfn::SyncedObject>( player ) },
		m_position{ this, player.m_position },
		m_velocity{ this, player.m_velocity },
		m_acceleration{ this, player.m_acceleration },
		m_rotation{ this, player.m_rotation },
		m_rotational_velocity{ this, player.m_rotational_velocity },
		m_color{ this, player.m_color }
	{
	}

	void Update() {
		// No command processing here... clients are not authoritative.

		// Our super complicated physics engine. Copy & Pasted from server.
		auto delta = m_update_clock.restart().asSeconds();

		m_rotation += m_rotational_velocity * delta;

		auto new_acceleration = sf::Vector2f{ std::cos( m_rotation ), -std::sin( m_rotation ) };
		new_acceleration *= m_acceleration;

		auto new_velocity = m_velocity + new_acceleration * delta;
		auto magnitude = std::sqrt( std::pow( new_velocity.x, 2.f ) + std::pow( new_velocity.y, 2.f ) );

		// Clamp to +100 units/sec.
		if( magnitude > 100.f ) {
			new_velocity /= magnitude;
			new_velocity *= 100.f;
		}

		// Simulate friction.
		if( new_acceleration == sf::Vector2f{ 0.f, 0.f } ) {
			new_velocity *= std::pow( .1f, delta );

			// Snap to 0 units/sec if low enough.
			if( magnitude < .5f ) {
				new_velocity = { 0.f, 0.f };
			}
		}

		if( m_velocity != new_velocity ) {
			m_velocity = new_velocity;
		}

		m_position += m_velocity * delta;

		static const auto pi = 3.14159265f;

		m_shape.setPosition( m_position );
		m_shape.setFillColor( m_color );
		m_shape.setRotation( m_rotation / pi * -180.f + 90.f );
	}

	void Draw( sf::RenderWindow& window ) const {
		window.draw( m_shape );
	}

protected:
	virtual object_type_id_type GetTypeID() const override {
		return type_id;
	}

private:
	sfn::SyncedType<sf::Vector2f> m_position;
	sfn::SyncedType<sf::Vector2f> m_velocity;
	sfn::SyncedFloat m_acceleration;
	sfn::SyncedFloat m_rotation;
	sfn::SyncedFloat m_rotational_velocity;
	sfn::SyncedType<sf::Color> m_color;

	sf::Clock m_update_clock{};

	sf::ConvexShape m_shape{ 3 };
};

const Player::object_type_id_type Player::type_id = 0x1337;

namespace sfn {

sfn::Message& operator<<( sfn::Message& message, const sf::Color& input ) {
	message << input.r << input.g << input.b << input.a;
	return message;
}

sfn::Message& operator>>( sfn::Message& message, sf::Color& output ) {
	message >> output.r >> output.g >> output.b >> output.a;
	return message;
}

sfn::Message& operator<<( sfn::Message& message, const sf::Vector2f& input ) {
	message << input.x << input.y;
	return message;
}

sfn::Message& operator>>( sfn::Message& message, sf::Vector2f& output ) {
	message >> output.x >> output.y;
	return message;
}

// Because sf::String doesn't have a const_iterator, it isn't a proper container... too bad.
sfn::Message& operator<<( sfn::Message& message, const sf::String& input ) {
	std::basic_string<sf::Uint32> str( input.getData() );
	message << str;
	return message;
}

sfn::Message& operator>>( sfn::Message& message, sf::String& output ) {
	std::basic_string<sf::Uint32> str;
	message >> str;
	output = sf::String( std::move( str ) );
	return message;
}

}

int main( int argc, char** argv ) {
	if( argc != 2 ) {
		std::cout << "Usage: " << argv[0] << " [address]\n";
		return -1;
	}

	sf::RenderWindow window( sf::VideoMode( 600, 400 ), "SFNUL Test Client", sf::Style::Titlebar | sf::Style::Close );

	window.setKeyRepeatEnabled( false );

	std::list<Player> players{};

	// Yes... here it is... legendary.
	std::list<Bullet> bullets{};

	ChatLog chat_log;

	// Support up to 1MiB blocks so that our font data can fit.
	sfn::SetMaximumBlockSize( 1024 * 1024 );

	auto addresses = sfn::IpAddress::Resolve( argv[1] );

	if( addresses.empty() ) {
		std::cout << "Could not resolve hostname \"" << argv[1] << "\" to an address.\n";
		return 1;
	}

	auto link = std::make_shared<sfn::Link<sfn::TcpSocket>>( sfn::TcpSocket::Create() );

	link->Connect( sfn::Endpoint{ addresses.front(), 31337 } );

	sfn::SynchronizerClient synchronizer;

	synchronizer.SetLifetimeManagers( Player::type_id,
		// Signature:
		// sfn::SyncedObject* Factory()
		[&players]() {
			players.emplace_back();
			return &players.back();
		},

		// Signature:
		// void Destructor( sfn::SyncedObject* )
		[&players]( sfn::SyncedObject* player ) {
			auto iter = std::find_if( std::begin( players ), std::end( players ),
				[player]( const Player& element ) {
					return player == &element;
				}
			);

			if( iter != std::end( players ) ) {
				players.erase( iter );
			}
		}
	);

	synchronizer.SetLifetimeManagers( ChatLog::type_id,
		// Signature:
		// sfn::SyncedObject* Factory()
		[&chat_log]() {
			return &chat_log;
		},

		// Signature:
		// void Destructor( sfn::SyncedObject* )
		[]( sfn::SyncedObject* ) {
		}
	);

	synchronizer.SetLifetimeManagers( Bullet::type_id,
		// Signature:
		// sfn::SyncedObject* Factory()
		[&bullets]() {
			bullets.emplace_back();
			return &bullets.back();
		},

		// Signature:
		// void Destructor( sfn::SyncedObject* )
		[&bullets]( sfn::SyncedObject* player ) {
			auto iter = std::find_if( std::begin( bullets ), std::end( bullets ),
				[player]( const Bullet& element ) {
					return player == &element;
				}
			);

			if( iter != std::end( bullets ) ) {
				bullets.erase( iter );
			}
		}
	);

	sfn::Start();

	auto connected = false;
	auto chatting = false;
	auto font_received = false;

	sf::String current_message;
	sf::String player_name;

	while( window.isOpen() ) {
		sf::Event event;

		while( window.pollEvent( event ) ) {
			if( event.type == sf::Event::Closed ) {
				link->Shutdown();
				connected = false;
				window.close();
				break;
			} else if( ( event.type == sf::Event::KeyPressed ) && ( event.key.code == sf::Keyboard::Return ) ) {
				chatting = !chatting;

				// Very secure and robust chat system.... not.
				if( !chatting ) {
					window.setKeyRepeatEnabled( false );

					if( player_name.isEmpty() && !current_message.isEmpty() ) {
						player_name = current_message;
					}
					else if( !current_message.isEmpty() ) {
						sfn::Message chat_message;

						chat_message << player_name + ": " + current_message;

						link->Send( StreamID::PlayerChatStream, chat_message );
					}

					current_message.clear();
				}
				else {
					window.setKeyRepeatEnabled( true );
				}
			} else if( ( event.type == sf::Event::TextEntered ) && chatting ) {
				if( ( event.text.unicode == 8 ) && current_message.getSize() ) { // Backspace
					current_message.erase( current_message.getSize() - 1, 1 );
				}
				else if( std::isprint( event.text.unicode ) ) {
					current_message += event.text.unicode;
				}
			} else if( ( event.type == sf::Event::KeyPressed ) && connected && !chatting ) {
				sfn::Message command_message;

				switch( event.key.code ) {
					case sf::Keyboard::W: {
						command_message << PlayerCommand::Accelerate;
						break;
					}
					case sf::Keyboard::A: {
						command_message << PlayerCommand::LeftStart;
						break;
					}
					case sf::Keyboard::Space: {
						command_message << PlayerCommand::StartFireWeapon;
						break;
					}
					case sf::Keyboard::D: {
						command_message << PlayerCommand::RightStart;
						break;
					}
					default: {
						break;
					}
				}

				if( command_message.GetSize() ) {
					link->Send( StreamID::PlayerCommandStream, command_message );
				}
			} else if( ( event.type == sf::Event::KeyReleased ) && connected && !chatting ) {
				sfn::Message command_message;

				switch( event.key.code ) {
					case sf::Keyboard::W: {
						command_message << PlayerCommand::Decelerate;
						break;
					}
					case sf::Keyboard::A: {
						command_message << PlayerCommand::LeftStop;
						break;
					}
					case sf::Keyboard::Space: {
						command_message << PlayerCommand::StopFireWeapon;
						break;
					}
					case sf::Keyboard::D: {
						command_message << PlayerCommand::RightStop;
						break;
					}
					default: {
						break;
					}
				}

				if( command_message.GetSize() ) {
					link->Send( StreamID::PlayerCommandStream, command_message );
				}
			}
		}

		if( !connected && link->GetTransport() && link->GetTransport()->IsConnected() ) {
			synchronizer.AddServer( link );
			connected = true;
		}

		if( connected && !font_received ) {
			sfn::Message font_data_message;

			if( link->Receive( StreamID::FontDataStream, font_data_message ) ) {
				// Receive the lovely font from the server.
				std::string font_data;
				font_data_message >> font_data;
				ChatLog::SetFontData( std::move( font_data ) );

				font_received = true;
			}
		}

		auto transport = link->GetTransport();

		if( connected && ( !transport || !transport->IsConnected() || transport->RemoteHasShutdown() ) ) {
			link->Shutdown();
			connected = false;
			break;
		}

		synchronizer.Update();

		// Update all players.
		for( auto& player : players ) {
			player.Update();
		}

		// Update all bullets.
		for( auto& bullet : bullets ) {
			bullet.Update();
		}

		window.clear();

		for( const auto& player : players ) {
			player.Draw( window );
		}

		for( const auto& bullet : bullets ) {
			bullet.Draw( window );
		}

		chat_log.Draw( window );

		if( chatting ) {
			chat_log.DrawCurrentMessage( window, current_message, !player_name.isEmpty() );
		}

		window.display();

		sf::sleep( sf::milliseconds( 20 ) );
	}

	sf::sleep( sf::milliseconds( 20 ) );

	sfn::Stop();

	return 0;
}
