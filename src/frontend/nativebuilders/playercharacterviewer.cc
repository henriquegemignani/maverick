
#include "frontend/nativebuilders/playercharacterviewer.h"

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

PlayerCharacterViewer::PlayerCharacterViewer(backend::PlayerCharacter* player_character)
    : player_character_(player_character)
    , primitive_(nullptr, nullptr)
{
    graphic::PrimitiveSetup::Sprite::Prepare(primitive_, resource::GetTextureAtlasFromFile("repo"));
    controller_ = dynamic_cast<ugdk::graphic::PrimitiveControllerSprite*>(primitive_.controller().get());

    player_character_->player().set_frame_change_callback([this](const ugdk::action::SpriteAnimationFrame& frame) {
        controller_->ChangeToAnimationFrame(frame);
    });
}

void PlayerCharacterViewer::Render(ugdk::graphic::Canvas & canvas) const
{
    ugdk::graphic::TextureUnit sprite_unit = ugdk::graphic::manager()->ReserveTextureUnit(primitive_.texture());
    canvas.PushAndCompose(math::Geometry(player_character_->position(), math::Vector2D(player_character_->direction(), 1.0)));
    canvas.PushAndCompose(math::Vector2D(-32, -56.0));
    canvas.SendUniform("drawable_texture", sprite_unit);
    primitive_.drawfunction()(primitive_, canvas);
    canvas.PopGeometry();
    canvas.PopGeometry();
}

} // namespace frontend