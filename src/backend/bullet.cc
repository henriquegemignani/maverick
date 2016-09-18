
#include "backend/bullet.h"

#include "backend/serverproxy.h"
#include "backend/playercharacter.h"
#include <ugdk/action/scene.h>

namespace backend {
  
using namespace ugdk;

namespace {
double kBulletLv0Speed = 8.0;
double kBulletLv1Speed = 10.0;
int kTicksToLive = 60;

std::tuple<std::string, double> data_for(Bullet::Type type) {
	switch (type) {
	case Bullet::Type::X1_LV0:
		return std::make_tuple("x1_lv0_", kBulletLv0Speed);
	case Bullet::Type::X1_LV1:
		return std::make_tuple("x1_lv1_", kBulletLv1Speed);
	default:
		throw std::exception("Unuspported bullet type.");
	}
}
}


Bullet::Bullet(const ugdk::math::Vector2D& position, Type type)
	: AnimatedObject("animations/buster.json", position)
    , finished_(false)
	, type_(type)
	, time_alive_(0)
	, state_(State::START)
{
    player_.AddObserver(this);

	if (type_ == Type::X1_LV0)
		state_ = State::LOOP;

	player_offset_x_ = position_.x - ServerProxy::reference()->player_character().position().x;
	first_update_ = true;
}


void Bullet::Update()
{
	player_.Update(1.0 / 60.0);

	auto data = data_for(type_);

	switch (state_) {
	case State::START:
		player_.Select(std::get<0>(data) + "start");
		position_.x = ServerProxy::reference()->player_character().position().x + player_offset_x_;
		break;
	case State::PRELOOP:		
	case State::LOOP:
		if (state_ == State::PRELOOP)
			player_.Select(std::get<0>(data) + "preloop");
		else
			player_.Select(std::get<0>(data) + "loop");
		
		if (!first_update_) {
			position_.x += direction_ * std::get<1>(data);
			if (++time_alive_ > kTicksToLive) {
				state_ = State::HIT;
			}
		}
		
		break;
	case State::HIT:
		player_.Select(std::get<0>(data) + "hit");
		break;
	default: break;
	}

	first_update_ = false;
}

void Bullet::Tick() {
	switch (state_) {
	case State::START:
		state_ = State::PRELOOP;
		break;
	case State::PRELOOP:
		state_ = State::LOOP;
		break;
	case State::LOOP:
		break;
	case State::HIT:
		finished_ = true;
		break;
	default: break;
	}
}

} // namespace frontend
