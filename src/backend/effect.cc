
#include "backend/effect.h"

#include <ugdk/action/scene.h>
#include <ugdk/resource/module.h>

namespace backend {
  
using namespace ugdk;

namespace {
std::string animation_table_name_for_type(Effect::Type type) {
	switch (type) {
	case Effect::Type::DUST:
		return "animations/dust.json";
	case Effect::Type::DASH_DUST:
		return "animations/dash_dust.json";
	default:
		throw std::exception("Unknown type.");
	}
}
}


Effect::Effect(const ugdk::math::Vector2D& position, Type type)
    : position_(position)
    , player_(ugdk::resource::GetSpriteAnimationTableFromFile(animation_table_name_for_type(type)))
    , finished_(false)
	, type_(type)
	, direction_(1)
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