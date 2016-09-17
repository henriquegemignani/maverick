
#include "backend/playercharacter.h"

#include <ugdk/action/scene.h>
#include <ugdk/resource/module.h>
#include <ugdk/input/events.h>
#include <ugdk/input/module.h>
#include <ugdk/input/joystick.h>
#include <algorithm>

#include "backend/serverproxy.h"

namespace backend {
  
using namespace ugdk;

namespace {
	bool get_bool_property(const tiled::PropertyMap& map, const std::string& name) {
		auto f = map.find(name);
		if (f != map.end()) {
			return f->second.bool_value();	
		}
		return false;
	}
}

PlayerCharacter::PlayerCharacter(ServerProxy* server)
    : position_(64.0, -16.0)
	, was_on_ground_(false)
    , on_ground_(false)
    , direction_(1.0)
    , player_(ugdk::resource::GetSpriteAnimationTableFromFile("x.json"))
	, server_(server)
{
    player_.AddObserver(this);
    player_.Select("warpin");
    player_.Refresh();
    state_ = AnimationState::WARPING;
}

void PlayerCharacter::Update(double dt)
{
	was_on_ground_ = on_ground_;
    GetPlayerInput();
    ApplyGravity();
    ApplyVelocity();
    CheckCollision();
   
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
                velocity_.y = -5.0;
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
	default:
		break;
    }
}

void PlayerCharacter::GetPlayerInput() {
    if (state_ == AnimationState::STANDING) {
		double x_axis = 0.0;
        if (auto joystick = current_joystick_.lock()) {
            x_axis = joystick->GetAxisStatus(0).Percentage();
        } else
        {
			const auto& keyboard = ugdk::input::manager()->keyboard();
			if (keyboard.IsDown(input::Scancode::D)) {
				x_axis = 1.0;
			} else if (keyboard.IsDown(input::Scancode::A)) {
				x_axis = -1.0;
			} else {
				x_axis = 0.0;
			}

			if (keyboard.IsPressed(input::Scancode::SPACE))
			{
				if (on_ground_) {
					on_ground_ = false;
					velocity_.y = -5.0;
				}
			}
        }
		if (abs(x_axis) > 0.2) {
			direction_ = x_axis / abs(x_axis);
			velocity_.x = direction_ * 1.5;
			if (on_ground_) {
				player_.Select("walk");
			}
		}
		else {
			velocity_.x = 0.0;
			if (on_ground_) {
				player_.Select("stand");
			}
		}
    }
}

void PlayerCharacter::ApplyGravity()
{
    velocity_.y += 0.25;
    velocity_.y = std::min(velocity_.y, 5.75);
}

void PlayerCharacter::ApplyVelocity()
{
	position_.x += velocity_.x / 60;
	CheckCollision();
	position_.y += velocity_.y / 60;
	CheckCollision();
}

void PlayerCharacter::CheckCollision() {
	auto map = server_->map();
	auto& layer = map->layers()[4];
	
	int tile_col = static_cast<int>(position_.x / map->tile_width());
	int tile_row = static_cast<int>(position_.y / map->tile_height());
	if (layer.IsInside(tile_col, tile_row))
	{
		auto tile = layer.tile_at(tile_col, tile_row);
		auto& properties = map->tileproperties_for(tile);
		if (get_bool_property(properties, "solid")) {
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
}

} // namespace frontend