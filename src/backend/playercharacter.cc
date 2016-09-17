
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
	struct TileCoords
	{
		int col, row;

		TileCoords(int c, int r) : col(c), row(r) {};

		TileCoords(const tiled::Map* map, double x, double y) 
			: col(static_cast<int>(x / map->tile_width()))
			, row(static_cast<int>(y / map->tile_height()))
		{}
	};
	bool get_bool_property(const tiled::PropertyMap& map, const std::string& name) {
		auto f = map.find(name);
		if (f != map.end()) {
			return f->second.bool_value();	
		}
		return false;
	}

	bool is_solid(const tiled::Map* map, const TileCoords& tile_pos)
	{
		auto& layer = map->layers()[0];
		if (layer.IsInside(tile_pos.col, tile_pos.row))
		{
			auto tile = layer.tile_at(tile_pos.col, tile_pos.row);
			auto& properties = map->tileproperties_for(tile);
			return get_bool_property(properties, "solid");
		}
		return false;
	}

	double tile_corner_x(const tiled::Map* map, const TileCoords& tile_pos, double direction)
	{
		int x = tile_pos.col;
		if (direction > 0)
			x += 1;
		return x * map->tile_width();
	}

	double tile_corner_y(const tiled::Map* map, const TileCoords& tile_pos, double direction)
	{
		int y = tile_pos.row;
		if (direction > 0)
			y += 1;
		return y * map->tile_height();
	}
}

PlayerCharacter::PlayerCharacter(ServerProxy* server)
    : position_(64.0, -16.0)
	, was_on_ground_(false)
    , on_ground_(false)
    , direction_(1.0)
    , player_(ugdk::resource::GetSpriteAnimationTableFromFile("x.json"))
	, server_(server)
	, width_(8)
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
		if (std::abs(x_axis) > 0.2) {
			direction_ = x_axis / std::abs(x_axis);
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
	auto map = server_->map();

	// Movement in X

	auto cornerX = position_.x + direction_ * width_ / 2;
	auto cornerXwithVel = cornerX + velocity_.x;

	TileCoords tile_pos_x(map, cornerXwithVel, position_.y);
	double x_to_move = velocity_.x;
	if (is_solid(map, tile_pos_x)) {
		double tile_corner = tile_corner_x(map, tile_pos_x, -direction_);
		double tile_distance = std::abs(tile_corner - cornerX);
		x_to_move = direction_ * std::min(tile_distance, std::abs(velocity_.x));
	}
	position_.x += x_to_move;

	// Movement in Y

	auto yDirection = velocity_.y / std::abs(velocity_.y);
	auto cornerY = position_.y + yDirection * 0.5;
	auto cornerYwithVel = cornerY + velocity_.y;

	TileCoords tile_pos_y(map, position_.x, cornerYwithVel);
	double y_to_move = velocity_.y;
	if (is_solid(map, tile_pos_y)) {
		double tile_corner = tile_corner_y(map, tile_pos_y, -yDirection);
		double tile_distance = std::abs(tile_corner - cornerY);
		y_to_move = yDirection * std::min(tile_distance, std::abs(velocity_.y));

		on_ground_ = true;
		velocity_.y = 0.0;
		if (state_ == AnimationState::WARPING) {
			state_ = AnimationState::WARP_FINISH;
			player_.Select("warp");
		}
	}
	position_.y += y_to_move;
}

void PlayerCharacter::CheckCollision(bool vertical) {
	auto map = server_->map();

	TileCoords tile_pos(map, position_.x, position_.y);
	if (is_solid(map, tile_pos)) {
		if (vertical) {
			position_.y = tile_pos.row * map->tile_height();
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