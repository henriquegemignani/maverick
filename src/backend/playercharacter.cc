
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

	template <typename T> int sgn(T val) {
		return (T(0) < val) - (val < T(0));
	}

	const double kGravity = 0.25;
	const double kTerminalSpeed = 5.75;
	const double kJumpSpeed = 5.0;
	const double kWalkingSpeed = 1.5;
	const double kWallSlidingSpeed = 2.0;
}

PlayerCharacter::PlayerCharacter(ServerProxy* server)
    : position_(64.0, -16.0)
	, was_on_ground_(false)
    , on_ground_(false)
	, on_wall_(false)
    , direction_(1)
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
    GetPlayerInput();

	if (IsAcceptingMovementInput()) {
		if (std::abs(input_x_axis_) > 0.2) {
			direction_ = sgn(input_x_axis_);
			velocity_.x = direction_ * kWalkingSpeed;
			if (state_ == AnimationState::STANDING)
				state_ = AnimationState::WALKING;
		}
		else {
			velocity_.x = 0.0;
			if (state_ == AnimationState::WALKING)
				state_ = AnimationState::STANDING;
		}
	}
	Jump();

    ApplyGravity();
    ApplyVelocity();
	UpdateAnimation();
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
    if (ev.button == 11) {
		should_jump_ = true;
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
        break;
	case AnimationState::WALLKICKING:
		state_ = AnimationState::ON_AIR;
		break;
	default:
		break;
    }
}

void PlayerCharacter::GetPlayerInput() {
    if (auto joystick = current_joystick_.lock()) {
		input_x_axis_ = joystick->GetAxisStatus(0).Percentage();

    } else {
		const auto& keyboard = ugdk::input::manager()->keyboard();
		if (keyboard.IsDown(input::Scancode::D)) {
			input_x_axis_ = 1.0;
		} else if (keyboard.IsDown(input::Scancode::A)) {
			input_x_axis_ = -1.0;
		} else {
			input_x_axis_ = 0.0;
		}

		if (keyboard.IsPressed(input::Scancode::SPACE))
			should_jump_ = true;
    }
}

void PlayerCharacter::ApplyGravity()
{
    velocity_.y += kGravity;
    velocity_.y = std::min(velocity_.y, kTerminalSpeed);
}

void PlayerCharacter::ApplyVelocity()
{
	auto map = server_->map();

	// Movement in X
	auto xDirection = sgn(velocity_.x);
	auto cornerX = position_.x + xDirection * width_ / 2;
	auto cornerXwithVel = cornerX + velocity_.x;

	TileCoords tile_pos_x(map, cornerXwithVel, position_.y);
	double x_to_move = velocity_.x;
	if (is_solid(map, tile_pos_x)) {
		double tile_corner = tile_corner_x(map, tile_pos_x, -xDirection);
		double tile_distance = std::abs(tile_corner - cornerX);
		x_to_move = xDirection * std::min(tile_distance, std::abs(velocity_.x));

		if (!on_ground()) {
			//direction_ = -xDirection;
			state_ = AnimationState::WALLSLIDING;
			velocity_.y = kWallSlidingSpeed;
		}
	} else if (state_ == AnimationState::WALLSLIDING) {
		state_ = AnimationState::ON_AIR;
	}
	position_.x += x_to_move;

	// Movement in Y
	auto yDirection = sgn(velocity_.y);
	auto cornerY = position_.y + yDirection * 0.5;
	auto cornerYwithVel = cornerY + velocity_.y;

	TileCoords tile_pos_y(map, position_.x, cornerYwithVel);
	double y_to_move = velocity_.y;
	if (is_solid(map, tile_pos_y)) {
		double tile_corner = tile_corner_y(map, tile_pos_y, -yDirection);
		double tile_distance = std::abs(tile_corner - cornerY);
		y_to_move = yDirection * std::min(tile_distance, std::abs(velocity_.y));

		if (!on_ground())
			Land();
	}
	position_.y += y_to_move;
}

void PlayerCharacter::Jump() {
	if (!should_jump_) return;
	should_jump_ = false;

	switch(state_)
	{
	case AnimationState::STANDING:
	case AnimationState::WALKING:
	case AnimationState::DASHING:
		state_ = AnimationState::ON_AIR;
		velocity_.y = -kJumpSpeed;
		break;

	case AnimationState::WALLSLIDING:
		// TODO: wallkick
		state_ = AnimationState::WALLKICKING;
		velocity_.y = -kJumpSpeed;
		direction_ = -direction_;
		velocity_.x = direction_ * kWalkingSpeed;
		break;

	default:
		// Can't jump!
		break;
	}
}

void PlayerCharacter::Land() {
	velocity_.y = 0.0;

	switch (state_)
	{
	case AnimationState::WARPING:
		state_ = AnimationState::WARP_FINISH;
		player_.Select("warp");
		break;

	case AnimationState::ON_AIR:
	case AnimationState::WALLSLIDING:
	case AnimationState::WALLKICKING:
		state_ = AnimationState::STANDING;
		break;

	default:
		break;
	}
}

void PlayerCharacter::UpdateAnimation()
{
	switch (state_)
	{
	case AnimationState::WARPING: break;
	case AnimationState::WARP_FINISH: break;
	case AnimationState::STANDING:
		player_.Select("stand");
		break;
	case AnimationState::WALKING:
		player_.Select("walk");
		break;
	case AnimationState::ON_AIR:
		if (velocity_.y < 0) {
			player_.Select("jump");
		} else {
			player_.Select("fall");
		}
		break;
	case AnimationState::DASHING: break;
	case AnimationState::WALLSLIDING:
		player_.Select("wallslide");
		break;
	case AnimationState::WALLKICKING:
		player_.Select("walljump");
		break;
	default: break;
	}
}

bool PlayerCharacter::on_ground() const
{
	switch (state_)
	{
	case AnimationState::STANDING:
	case AnimationState::WALKING:
	case AnimationState::DASHING:
	case AnimationState::WARP_FINISH:
		return true;
		break;

	default:
		return false;
		break;
	}
}

bool PlayerCharacter::IsAcceptingMovementInput() const
{
	switch (state_)
	{
	case AnimationState::STANDING:
	case AnimationState::WALKING:
	case AnimationState::ON_AIR:
	case AnimationState::WALLSLIDING:
		return true;

	case AnimationState::WARPING:
	case AnimationState::WARP_FINISH:
	case AnimationState::DASHING:
	case AnimationState::WALLKICKING:
	default:
		return false;
	}
}

} // namespace frontend