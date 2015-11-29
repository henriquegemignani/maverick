
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
    , position_(64.0, -16.0)
    , on_ground_(false)
    , direction_(1.0)
    , player_(ugdk::resource::GetSpriteAnimationTableFromFile("x.json"))
{
    graphic::PrimitiveSetup::Sprite::Prepare(primitive_, resource::GetTextureAtlasFromFile("repo"));
    controller_ = dynamic_cast<ugdk::graphic::PrimitiveControllerSprite*>(primitive_.controller().get());

    player_.set_frame_change_callback([this](const ugdk::graphic::SpriteAnimationFrame& frame) {
        controller_->ChangeToAnimationFrame(frame);
    });
    player_.AddObserver(this);

    player_.Select("warpin");
    player_.Refresh();
    state_ = AnimationState::WARPING;
}

void PlayerCharacter::Update(double dt)
{
    if (state_ == AnimationState::STANDING) {
        if (auto joystick = current_joystick_.lock()) {
            double x_axis = joystick->GetAxisStatus(0).Percentage();
            if (abs(x_axis) > 0.2) {
                direction_ = x_axis / abs(x_axis);
                position_.x += x_axis * 60 * dt;
                if (on_ground_) {
                    player_.Select("walk");
                }
            }
            else {
                if (on_ground_) {
                    player_.Select("stand");
                }
            }
        }
    }
    position_ += velocity_ * dt;
    velocity_.y += 512 * dt;
    velocity_.y = std::min(velocity_.y, 512.0);
    if (position_.y > 88.0) {
        position_.y = 88.0;
        velocity_.y = 0.0;
        if (!on_ground_ && state_ == AnimationState::STANDING) {
            //
        }
        on_ground_ = true;
        if (state_ == AnimationState::WARPING) {
            state_ = AnimationState::WARP_FINISH;
            player_.Select("warp");
        }
    }
    if (!on_ground_ && state_ == AnimationState::STANDING)
    {
        if (velocity_.y < 0)
        {
            player_.Select("jump");
        }
        else {
            player_.Select("fall");
        }
    }
    player_.Update(dt);
}

void PlayerCharacter::Render(ugdk::graphic::Canvas & canvas) const
{
    ugdk::graphic::TextureUnit sprite_unit = ugdk::graphic::manager()->ReserveTextureUnit(primitive_.texture());
    canvas.PushAndCompose(graphic::Geometry(position_, math::Vector2D(direction_, 1.0)));
    canvas.PushAndCompose(math::Vector2D(-32, 0));
    canvas.SendUniform("drawable_texture", sprite_unit);
    primitive_.drawfunction()(primitive_, canvas);
    canvas.PopGeometry();
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
    if (state_ == AnimationState::STANDING) {
        if (ev.button == 11) {
            if (on_ground_) {
                on_ground_ = false;
                velocity_.y = -320.0;
            }
        }
    }
}

void PlayerCharacter::Handle(const ugdk::input::JoystickButtonReleasedEvent& ev) {

}

void PlayerCharacter::Tick() {
    switch (state_) {
    case AnimationState::WARPING:        
        break;
    case AnimationState::WARP_FINISH:
        state_ = AnimationState::STANDING;
        player_.Select("stand");
        break;
    case AnimationState::STANDING:
        break;
    }
}

} // namespace frontend