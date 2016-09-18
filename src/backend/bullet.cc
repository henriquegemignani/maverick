
#include "backend/bullet.h"

#include <ugdk/action/scene.h>
#include <ugdk/resource/module.h>

namespace backend {
  
using namespace ugdk;

namespace {
}


Bullet::Bullet(const ugdk::math::Vector2D& position, Type type)
	: AnimatedObject("animations/buster.json", position)
    , finished_(false)
	, type_(type)
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