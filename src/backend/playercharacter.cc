
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

    const int kJumpJoystickKey = 11;
	const int kDashJoystickKey = 12;
    const int kShootJoystickKey = 13;

	const double kGravity = 0.25;
	const double kTerminalSpeed = 5.75;
	const double kJumpSpeed = 5.0;
	const double kWalkingSpeed = 1.5;
	const double kWallSlidingSpeed = 1.0;
    const double kWallKickJumpSpeed = 3.0;
	const double kDashingSpeed = 3.5;
	const int kDashLength = 45;
	const int kShootAnimationLength = 16;
}

PlayerCharacter::PlayerCharacter(ServerProxy* server)
    : position_(64.0, -16.0)
    , direction_(1)
    , player_(ugdk::resource::GetSpriteAnimationTableFromFile("animations/x.json"))
	, server_(server)
	, width_(8)
	, shoot_anim_ticks_(kShootAnimationLength)
    , show_pre_walk_(false)
    , show_wall_touch_(false)
    , show_wallkick_start_(false)
    , show_jump_recoil_(false)
{
    player_.AddObserver(this);
    player_.Select("warpin");
    player_.Refresh();
    state_ = AnimationState::WARPING;
}

void PlayerCharacter::Update(double dt)
{
    GetPlayerInput();

    if (state_ == AnimationState::DASHING) {
        if (std::abs(input_x_axis_) > 0.2 && sgn(input_x_axis_) != direction_)
            state_ = AnimationState::WALKING;
    }

	if (IsAcceptingMovementInput()) {
		if (std::abs(input_x_axis_) > 0.2) {
			direction_ = sgn(input_x_axis_);
			velocity_.x = direction_ * kWalkingSpeed;
			if (state_ == AnimationState::STANDING) {
				show_pre_walk_ = true;
				state_ = AnimationState::WALKING;
			}
		} else {
			velocity_.x = 0.0;
			if (state_ == AnimationState::WALKING)
				state_ = AnimationState::STANDING;
		}
	}

    if (state_ == AnimationState::ON_AIR) {
        if (velocity_.y < 0.0 && !holding_jump_)
            velocity_.y = 0.0;
    }

	Dash();
	Jump();

    shoot_anim_ticks_++;

    if (should_shoot_) {
        should_shoot_ = false;
        shoot_anim_ticks_ = 0;
    }

    ApplyGravity();
    ApplyVelocity();
	UpdateAnimation();
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
    if (ev.button == kJumpJoystickKey) {
		should_jump_ = true;
    }
	if (ev.button == kDashJoystickKey)
		should_dash_ = true;

    if (ev.button == kShootJoystickKey)
        should_shoot_ = true;
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
    case AnimationState::WALLSLIDING:
        show_wall_touch_ = false;
        break;
	case AnimationState::WALLKICKING:
        if (show_wallkick_start_) {
            show_wallkick_start_ = false;
            velocity_.y = -kWallKickJumpSpeed;
            velocity_.x = -direction_ * kWalkingSpeed;
        } else {
            state_ = AnimationState::ON_AIR;
        }
		break;
	case AnimationState::DASHING:
		dash_start_ = false;
		break;
	case AnimationState::WALKING:
		show_pre_walk_ = false;
        break;
    case AnimationState::STANDING:
        show_jump_recoil_ = false;
	default:
		break;
    }
}

void PlayerCharacter::GetPlayerInput() {
    if (auto joystick = current_joystick_.lock()) {
		input_x_axis_ = joystick->GetAxisStatus(0).Percentage();
        holding_jump_ = joystick->IsDown(kJumpJoystickKey);
		holding_dash_ = joystick->IsDown(kDashJoystickKey);

    } else {
		const auto& keyboard = ugdk::input::manager()->keyboard();
		if (keyboard.IsDown(input::Scancode::D) || keyboard.IsDown(input::Scancode::RIGHT)) {
			input_x_axis_ = 1.0;
		} else if (keyboard.IsDown(input::Scancode::A) || keyboard.IsDown(input::Scancode::LEFT)) {
			input_x_axis_ = -1.0;
		} else {
			input_x_axis_ = 0.0;
		}

        holding_jump_ = keyboard.IsDown(input::Scancode::SPACE);
		if (keyboard.IsPressed(input::Scancode::SPACE))
			should_jump_ = true;

		holding_dash_ = keyboard.IsDown(input::Scancode::LSHIFT);
		if (keyboard.IsPressed(input::Scancode::LSHIFT))
			should_dash_ = true;

        if (keyboard.IsPressed(input::Scancode::Z))
            should_shoot_ = true;
    }
}

void PlayerCharacter::ApplyGravity()
{
    if (state_ != AnimationState::WALLKICKING) {
        velocity_.y += kGravity;
        velocity_.y = std::min(velocity_.y, kTerminalSpeed);        
    }
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

        if (on_ground()) {
            velocity_.x = 0.0;
            state_ = AnimationState::STANDING;

        } else if (velocity_.y > 0.0) {
		    if (state_ != AnimationState::WALLSLIDING) {
                show_wall_touch_ = true;
		    }
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

		velocity_.y = 0.0;
		if (!on_ground())
			Land();
	} else if (on_ground()) {
		state_ = AnimationState::ON_AIR;
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
		state_ = AnimationState::WALLKICKING;
        show_wallkick_start_ = true;
        velocity_.y = 0.0;
		break;

	default:
		// Can't jump!
		break;
	}
}

void PlayerCharacter::Dash()
{
	switch (state_)
	{
	case AnimationState::STANDING:
	case AnimationState::WALKING:
		if (should_dash_) {
			state_ = AnimationState::DASHING;
			velocity_.x = direction_ * kDashingSpeed;
			dash_ticks_ = 0;
			dash_start_ = true;
		}
		break;

	case AnimationState::ON_AIR:
		// TODO: air dash!
		break;

	case AnimationState::DASHING:
		dash_ticks_++;
		if (!holding_dash_ || dash_ticks_ >= kDashLength)
			state_ = AnimationState::STANDING;
		break;

	default:
		// Can't dash!
		break;
	}
	should_dash_ = false;
}

void PlayerCharacter::Land() {
	switch (state_)
	{
	case AnimationState::WARPING:
		state_ = AnimationState::WARP_FINISH;
		player_.Select("warp");
		break;

	case AnimationState::ON_AIR:
        show_jump_recoil_ = true;
        // no break on purpose
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
    player_.Update(1.0 / 60.0);
	switch (state_)
	{
	case AnimationState::WARPING: break;
	case AnimationState::WARP_FINISH: break;
	case AnimationState::STANDING:
        if (show_jump_recoil_)
            ChangeAnimation("jumprecoil");
        else
            ChangeAnimation("stand");
		break;
	case AnimationState::WALKING:
		if (show_pre_walk_)
            ChangeAnimation("prewalk");
		else
            ChangeAnimation("walk");
		break;
	case AnimationState::ON_AIR:
		if (velocity_.y < 0) {
            ChangeAnimation("jump");
		} else {
            ChangeAnimation("fall");
		}
		break;
	case AnimationState::DASHING:
		if (dash_start_)
            ChangeAnimation("dashstart");
		else
            ChangeAnimation("dash");
		break;
	case AnimationState::WALLSLIDING:
        if (show_wall_touch_)
            ChangeAnimation("walltouch");
        else
            ChangeAnimation("wallslide");
		break;
	case AnimationState::WALLKICKING:
        if (show_wallkick_start_)
            ChangeAnimation("walljump_start");
        else
            ChangeAnimation("walljump_end");
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

void PlayerCharacter::ChangeAnimation(const std::string& animation_name) {
    bool restart = animation_name != current_animation_name_;
    if (shoot_anim_ticks_ < kShootAnimationLength) {
        player_.Select(animation_name + "_shoot", restart);
    } else {
        player_.Select(animation_name, restart);
    }
    current_animation_name_ = animation_name;
}

} // namespace frontend