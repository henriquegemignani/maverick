
#include "backend/bullet.h"

#include <ugdk/action/scene.h>

namespace backend {
  
using namespace ugdk;

namespace {
double kBulletSpeed = 8.0;
int kTicksToLive = 60;

std::string prefix_for(Bullet::Type type) {
	switch (type) {
	case Bullet::Type::X1_LV0:
		return "x1_lv0_";
	case Bullet::Type::X1_LV1:
		return "x1_lv1_";
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

	UpdateAnimation();
}


void Bullet::Update()
{
	switch (state_) {
	case State::LOOP:
		position_.x += direction_ * kBulletSpeed;

		if (++time_alive_ > kTicksToLive) {
			state_ = State::HIT;
		}
		break;
	default: break;
	}

    player_.Update(1.0 / 60.0);
	UpdateAnimation();
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

void Bullet::UpdateAnimation() {
	switch(state_) {
	case State::START:
		player_.Select(prefix_for(type_) + "start");
		break;
	case State::PRELOOP:
		player_.Select(prefix_for(type_) + "preloop");
		break;
	case State::LOOP:
		player_.Select(prefix_for(type_) + "loop");
		break;
	case State::HIT:
		player_.Select(prefix_for(type_) + "hit");
		break;
	default: break;
	}
}

} // namespace frontend