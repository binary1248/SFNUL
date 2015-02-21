/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <cmath>
#include <iostream>
#include <fstream>
#include <streambuf>
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
	SyncStream = 200 // DO NOT TOUCH STREAM IDS >= 200 They are used for internal purposes.
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

	void PushMessage( const sf::String& chat_message ) {
		// If you are only inspecting an object, remember to grab a const reference to it.
		// Failing to do so will soak up your transfer volume in no time.
		if( m_chat_messages->size() > 5 ) {
			m_chat_messages->pop_back();
		}

		m_chat_messages->push_front( std::move( chat_message ) );
	}

	// Singletons are a necessary evil to keep things simple here.
	static void SetInstance( ChatLog&& chatlog ) {
		m_instance.reset( new ChatLog( std::forward<ChatLog>( chatlog ) ) );
	}

	static ChatLog& GetInstance() {
		return *m_instance;
	}

protected:
	virtual object_type_id_type GetTypeID() const override {
		return type_id;
	}

private:
	sfn::SyncedType<std::deque<sf::String>> m_chat_messages;

	static std::unique_ptr<ChatLog> m_instance;
};

const ChatLog::object_type_id_type ChatLog::type_id = 0x1338;
std::unique_ptr<ChatLog> ChatLog::m_instance{};

// I was going to call this Projectile, but I figured I should pay homage to all those threads...
class Bullet : public sfn::SyncedObject {
public:
	const static object_type_id_type type_id;

	Bullet( const std::tuple<sf::Vector2f, sf::Vector2f, bool>& data ) :
		m_position{ this, sfn::SynchronizationType::Stream, std::get<0>( data ) },
		m_velocity{ this, sfn::SynchronizationType::Dynamic, std::get<1>( data ) }
	{
	}

	Bullet( Bullet&& bullet ) :
		sfn::SyncedObject{ std::forward<sfn::SyncedObject>( bullet ) },
		m_position{ this, bullet.m_position },
		m_velocity{ this, bullet.m_velocity }
	{
	}

	void Update() {
		// Our super complicated physics engine.
		auto delta = m_update_clock.restart().asSeconds();

		m_position += m_velocity * delta;

		m_shape.setPosition( m_position );
	}

	void Draw( sf::RenderWindow& window ) const {
		window.draw( m_shape );
	}

	bool IsActive() const {
		const static auto lifetime = sf::seconds( 1 );

		return m_life_clock.getElapsedTime() <= lifetime;
	}

protected:
	virtual object_type_id_type GetTypeID() const override {
		return type_id;
	}

private:
	sfn::SyncedType<sf::Vector2f> m_position;
	sfn::SyncedType<sf::Vector2f> m_velocity;

	sf::Clock m_update_clock{};
	sf::Clock m_life_clock{};

	sf::CircleShape m_shape{};
};

const Bullet::object_type_id_type Bullet::type_id = 0x1339;

// Player class representing a client-controllable entity.
class Player : public sfn::SyncedObject {
public:
	const static object_type_id_type type_id;

	Player( std::shared_ptr<sfn::Link<sfn::TcpSocket>> player_link ) :
		m_position{ this, sfn::SynchronizationType::Stream, { 300.f + position_dist( gen ), 200.f + position_dist( gen ) } },
		m_velocity{ this, sfn::SynchronizationType::Stream, { 0.f, 0.f } },
		m_acceleration{ this, sfn::SynchronizationType::Dynamic, 0.f },
		m_rotation{ this, sfn::SynchronizationType::Stream, 0.f },
		m_rotational_velocity{ this, sfn::SynchronizationType::Dynamic, 0.f },
		m_color{ this, sfn::SynchronizationType::Static, sf::Color{ dist( gen ), dist( gen ), dist( gen ), 255 } },
		m_link{ player_link }
	{
	}

	Player( Player&& player ) :
		sfn::SyncedObject{ std::forward<sfn::SyncedObject>( player ) },
		m_position{ this, player.m_position },
		m_velocity{ this, player.m_velocity },
		m_acceleration{ this, player.m_acceleration },
		m_rotation{ this, player.m_rotation },
		m_rotational_velocity{ this, player.m_rotational_velocity },
		m_color{ this, player.m_color },
		m_link{ std::move( player.m_link ) }
	{
	}

	~Player() {
		if( m_link && ( m_link->GetTransport() && m_link->GetTransport()->IsConnected() ) ) {
			m_link->Shutdown();
		}
	}

	void Update() {
		sfn::Message message;

		// Check if the player sent a chat message.
		while( m_link->Receive( StreamID::PlayerChatStream, message ) ) {
			sf::String chat_string;

			message >> chat_string;

			ChatLog::GetInstance().PushMessage( chat_string );
		}

		// Check if the player issued a command.
		while( m_link->Receive( StreamID::PlayerCommandStream, message ) ) {
			sfn::Uint8 command;
			message >> command;

			static const auto pi = 3.14159265f;

			switch( command ) {
				case PlayerCommand::LeftStart: {
					m_rotational_velocity = pi;
					break;
				}
				case PlayerCommand::LeftStop: {
					m_rotational_velocity = 0.f;
					break;
				}
				case PlayerCommand::RightStart: {
					m_rotational_velocity = -pi;
					break;
				}
				case PlayerCommand::RightStop: {
					m_rotational_velocity = 0.f;
					break;
				}
				case PlayerCommand::Accelerate: {
					m_acceleration = 100.f;
					break;
				}
				case PlayerCommand::Decelerate: {
					m_acceleration = 0.f;
					break;
				}
				case PlayerCommand::StartFireWeapon: {
					m_firing = true;
					break;
				}
				case PlayerCommand::StopFireWeapon: {
					m_firing = false;
					break;
				}
				default: {
					break;
				}
			}
		}

		// Our super complicated physics engine.
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
	}

	bool IsActive() const {
		auto transport = m_link->GetTransport();
		return transport && !transport->RemoteHasShutdown();
	}

	std::tuple<sf::Vector2f, sf::Vector2f, bool> FiredWeapon() {
		const static auto recoil = sf::milliseconds( 100 );
		const static auto muzzle_velocity = 200.f;

		if( !m_firing || ( m_recoil_clock.getElapsedTime() < recoil ) ) {
			return std::make_tuple( sf::Vector2f{}, sf::Vector2f{}, false );
		}

		m_recoil_clock.restart();

		auto velocity = sf::Vector2f{ std::cos( m_rotation ), -std::sin( m_rotation ) };
		velocity *= muzzle_velocity;

		auto position = sf::Vector2f{ m_position };

		return std::make_tuple( position, velocity, true );
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

	// Link used to communicate with the player.
	std::shared_ptr<sfn::Link<sfn::TcpSocket>> m_link;

	sf::Clock m_update_clock{};
	sf::Clock m_recoil_clock{};

	bool m_firing{ false };

	static std::mt19937 gen;
	static std::uniform_int_distribution<sf::Uint8> dist;
	static std::uniform_real_distribution<float> position_dist;
};

const Player::object_type_id_type Player::type_id = 0x1337;

std::mt19937 Player::gen{};
std::uniform_int_distribution<sf::Uint8> Player::dist{ 100, 255 };
std::uniform_real_distribution<float> Player::position_dist{ -180.f, 180.f };

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

int main() {
	std::fstream font{ "LiberationSans-Regular.ttf", std::fstream::in | std::fstream::binary };

	if( !font.is_open() ) {
		std::cout << "Could not open font file.\n";
		return -1;
	}

	std::string font_data( std::istreambuf_iterator<char>{ font }, std::istreambuf_iterator<char>{} );

	std::cout << "Loaded font data, " << font_data.size() << " bytes.\n";

	auto listener = sfn::TcpListener::Create();

	listener->Listen( sfn::Endpoint{ sfn::IpAddress{ "0.0.0.0" }, 31337 } );

	std::list<Player> players{};

	// Yes... here it is... legendary.
	std::list<Bullet> bullets{};

	// Support up to 1MiB blocks so that our font data can fit.
	sfn::SetMaximumBlockSize( 1024 * 1024 );

	sfn::SynchronizerServer synchronizer;

	sfn::SetStreamSynchronizationPeriod( std::chrono::milliseconds( 5000 ) );

	sfn::Start();

	ChatLog::SetInstance( synchronizer.CreateObject<ChatLog>() );

	auto link = std::make_shared<sfn::Link<sfn::TcpSocket>>();

	std::atomic_bool exit{ false };
	std::cout << "Press ENTER to exit.\n";
	sfn::Thread exit_handler{ [&]() { std::cin.get(); exit = true; } };

	while( !exit ) {
		// Accept incoming connection requests.
		while( true ) {
			link->SetTransport( listener->GetPendingConnection() );

			if( link->GetTransport() && link->GetTransport()->IsConnected() ) {
				// Seems like the link is active. Create a new player.
				// Remember to add the client to the synchronizer
				// before creating the player object.
				synchronizer.AddClient( link );
				players.emplace_back( synchronizer.CreateObject<Player>( link ) );

				// Send the client our lovely font.
				sfn::Message font_data_message;
				font_data_message << font_data;

				link->Send( StreamID::FontDataStream, font_data_message );

				link = std::make_shared<sfn::Link<sfn::TcpSocket>>();
			}
			else {
				break;
			}
		}

		// Update all players.
		for( auto iter = std::begin( players ); iter != std::end( players ); ) {
			if( !iter->IsActive() ) {
				iter = players.erase( iter );
				continue;
			}

			iter->Update();

			auto bullet_data = iter->FiredWeapon();

			if( std::get<2>( bullet_data ) ) {
				// We gotta make a new bullet!!1
				bullets.emplace_back( synchronizer.CreateObject<Bullet>( bullet_data ) );
			}

			++iter;
		}

		// Update all bullets.
		for( auto iter = std::begin( bullets ); iter != std::end( bullets ); ) {
			if( !iter->IsActive() ) {
				iter = bullets.erase( iter );
				continue;
			}

			iter->Update();

			++iter;
		}

		synchronizer.Update();

		sf::sleep( sf::milliseconds( 20 ) );
	}

	players.clear();
	bullets.clear();

	sf::sleep( sf::milliseconds( 20 ) );

	sfn::Stop();

	return 0;
}
