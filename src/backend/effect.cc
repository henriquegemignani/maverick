
#include "backend/effect.h"

#include <ugdk/action/scene.h>
#include <ugdk/resource/module.h>

namespace backend {
  
using namespace ugdk;


Effect::Effect(const ugdk::math::Vector2D& position, const std::string& effect_name)
    : position_(position)
    , player_(ugdk::resource::GetSpriteAnimationTableFromFile(effect_name))
    , finished_(false)
{
    player_.AddObserver(this);
    player_.Select("idle");
    player_.Refresh();
}


void Effect::Update()
{
    player_.Update(1.0 / 60.0);
}

void Effect::Tick() {
    finished_ = true;
}

} // namespace frontend