
#include "frontend/nativebuilders/effectviewer.h"

#include <ugdk/action/scene.h>
#include <ugdk/graphic/canvas.h>
#include <ugdk/graphic/module.h>
#include <ugdk/graphic/primitive.h>
#include <ugdk/graphic/textureatlas.h>
#include <ugdk/graphic/primitivesetup.h>
#include <ugdk/graphic/sprite.h>
#include <ugdk/resource/module.h>
#include <ugdk/ui/drawable/texturedrectangle.h>
#include <ugdk/input/events.h>
#include <ugdk/input/module.h>
#include <ugdk/input/joystick.h>
#include <algorithm>

namespace frontend {
  
using namespace ugdk;

EffectViewer::EffectViewer(backend::ServerProxy* server)
    : server_(server)
    , primitive_(nullptr, nullptr)
{
    graphic::PrimitiveSetup::Sprite::Prepare(primitive_, resource::GetTextureAtlasFromFile("spritesheets/dust"));
    controller_ = dynamic_cast<ugdk::graphic::PrimitiveControllerSprite*>(primitive_.controller().get());
}

void EffectViewer::Render(ugdk::graphic::Canvas & canvas) const
{
    auto sprite_unit = ugdk::graphic::manager()->ReserveTextureUnit(primitive_.texture());

    for (const auto& effect : server_->effects()) {
        controller_->ChangeToAnimationFrame(effect.player().current_animation_frame());
        canvas.PushAndCompose(math::Geometry(effect.position() + math::Vector2D(-8.0, -8.0)));
        canvas.SendUniform("drawable_texture", sprite_unit);
        primitive_.drawfunction()(primitive_, canvas);
        canvas.PopGeometry();
    }
}

} // namespace frontend