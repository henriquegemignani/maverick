
#include "backend/bullet.h"

#include <ugdk/action/scene.h>
#include <ugdk/resource/module.h>

namespace backend {
  
using namespace ugdk;

namespace {
std::string animation_table_name_for_type(Bullet::Type type) {
	switch (type) {
	case Bullet::Type::X1_LV0:
		return "animations/dust.json";
	case Bullet::Type::X1_LV1:
		return "animations/dash_dust.json";
	default:
		throw std::exception("Unknown type.");
	}
}
}


Bullet::Bullet(const ugdk::math::Vector2D& position, Type type)
    : position_(position)
    , player_(ugdk::resource::GetSpriteAnimationTableFromFile("animations/buster.json"))
    , finished_(false)
	, type_(type)
	, direction_(1)
{
    player_.AddObserver(this);
}


void Bullet::Update()
{
    player_.Update(1.0 / 60.0);
}

void Bullet::Tick() {
    finished_ = true;
}

} // namespace frontend