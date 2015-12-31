
#include "backend/playercharacter.h"

#include <ugdk/action/scene.h>
#include <ugdk/resource/module.h>
#include <ugdk/input/events.h>
#include <ugdk/input/module.h>
#include <ugdk/input/joystick.h>
#include <algorithm>

namespace backend {
  
using namespace ugdk;

PlayerCharacter::PlayerCharacter()
    : position_(64.0, -16.0)
    , on_ground_(false)
    , direction_(1.0)
    , player_(ugdk::resource::GetSpriteAnimationTableFromFile("x.json"))
{
    player_.AddObserver(this);
    player_.Select("warpin");
    player_.Refresh();
    state_ = AnimationState::WARPING;
}

void PlayerCharacter::Update(double dt)
{
    GetPlayerInput();
    ApplyGravity(dt);
    ApplyVelocity(dt);
    CheckGroundCollision();
   
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
                velocity_.y = -5.0 * 60;
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

void PlayerCharacter::GetPlayerInput() {
    if (state_ == AnimationState::STANDING) {
        if (auto joystick = current_joystick_.lock()) {
            double x_axis = joystick->GetAxisStatus(0).Percentage();
            if (abs(x_axis) > 0.2) {
                direction_ = x_axis / abs(x_axis);
                velocity_.x = direction_ * 1.5 * 60;
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
}

void PlayerCharacter::ApplyGravity(double dt)
{
    velocity_.y += 0.25 * 60 * 60 * dt;
    velocity_.y = std::min(velocity_.y, 5.75 * 60);
}

void PlayerCharacter::ApplyVelocity(double dt)
{
    position_ += velocity_ * dt;
}

void PlayerCharacter::CheckGroundCollision() {
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
}

} // namespace frontend