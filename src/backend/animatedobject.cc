
#include "backend/animatedobject.h"

#include <ugdk/action/scene.h>
#include <ugdk/resource/module.h>

namespace backend {
  
AnimatedObject::AnimatedObject(const std::string& animations_name, const ugdk::math::Vector2D& position)
    : AtlasObject(animations_name, position)
	, player_(ugdk::resource::GetSpriteAnimationTableFromFile(animations_name))
{}

} // namespace frontend