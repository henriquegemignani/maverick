
#include "frontend/nativebuilders/playercharacter.h"

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

PlayerCharacter::PlayerCharacter()
    : primitive_(nullptr, nullptr)
    , current_frame_(0)
{
    graphic::PrimitiveSetup::Sprite::Prepare(primitive_, resource::GetTextureAtlasFromFile("repo"));
    controller_ = dynamic_cast<ugdk::graphic::PrimitiveControllerSprite*>(primitive_.controller().get());

    controller_->ChangeToAtlasFrame(current_frame_++);
}

void PlayerCharacter::Update(double dt)
{
    auto joysticks = ugdk::input::manager()->CurrentJoysticks();
    if (!joysticks.empty()) {
        auto joystick = joysticks.front();
        double x_axis = joystick->GetAxisStatus(0).Percentage();
        if (abs(x_axis) > 0.2)
            position_.x += x_axis * 60 * dt;
    }
    position_ += velocity_ * dt;
    velocity_.y += 512 * dt;
    velocity_.y = std::min(velocity_.y, 512.0);
    if (position_.y > 96.0) {
        position_.y = 96.0;
        velocity_.y = 0.0;
        on_ground_ = true;
    }
}

void PlayerCharacter::Render(ugdk::graphic::Canvas & canvas) const
{
    ugdk::graphic::TextureUnit sprite_unit = ugdk::graphic::manager()->ReserveTextureUnit(primitive_.texture());
    canvas.PushAndCompose(graphic::Geometry(position_));
    canvas.SendUniform("drawable_texture", sprite_unit);
    primitive_.drawfunction()(primitive_, canvas);
    canvas.PopGeometry();
}

void PlayerCharacter::HandleNewJoystick(std::shared_ptr<ugdk::input::Joystick> joystick)
{
    joystick->event_handler().AddObjectListener(this);
    current_joystick_ = joystick;
}

void PlayerCharacter::Handle(const ugdk::input::JoystickDisconnectedEvent& ev) {
    if (auto joystick = ev.joystick.lock()) {
        joystick->event_handler().RemoveObjectListener(this);
    }
    current_joystick_.reset();
}

void PlayerCharacter::Handle(const ugdk::input::JoystickAxisEvent& ev) {

}

void PlayerCharacter::Handle(const ugdk::input::JoystickButtonPressedEvent& ev) {
    if (ev.button == 11) {
        if (on_ground_) {
            on_ground_ = false;
            velocity_.y = -256.0;
        }
    }
}

void PlayerCharacter::Handle(const ugdk::input::JoystickButtonReleasedEvent& ev) {

}

void PlayerCharacter::GoToNextFrame()
{
    controller_->ChangeToAtlasFrame(current_frame_++);
}

} // namespace frontend