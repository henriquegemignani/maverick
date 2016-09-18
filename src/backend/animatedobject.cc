
#include "backend/animatedobject.h"

#include <ugdk/action/scene.h>
#include <ugdk/resource/module.h>

namespace backend {
  
using namespace ugdk;


AnimatedObject::AnimatedObject(const std::string& animations_name, const ugdk::math::Vector2D& position)
    : direction_(1)
    , position_(position)
	, player_(ugdk::resource::GetSpriteAnimationTableFromFile(animations_name))
	, animations_name_(animations_name)
{}


} // namespace frontend