
#include "backend/playercharacter.h"

#include <ugdk/action/scene.h>
#include <algorithm>

#include "backend/serverproxy.h"
#include "backend/collision.h"

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

		bool IsInteriorX(const tiled::Map* map, const math::Vector2D& pos) const {
			return pos.x > col * map->tile_width()
				&& pos.x < (col + 1) * map->tile_width();
		}

		bool IsInteriorY(const tiled::Map* map, const math::Vector2D& pos) const {
			return pos.y > row * map->tile_height()
				&& pos.y < (row + 1) * map->tile_height();
		}
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

	double tile_corner_x(const tiled::Map* map, const TileCoords& tile_pos, int direction)
	{
		int x = tile_pos.col;
		if (direction > 0)
			x += 1;
		return x * map->tile_width();
	}

	double tile_corner_y(const tiled::Map* map, const TileCoords& tile_pos, int direction)
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
	const double kWallSlidingSpeed = 2.0;
    const double kWallKickJumpSpeed = 4.0;
	const double kDashingSpeed = 3.5;
	const int kDashLength = 33;
	const int kShootAnimationLength = 16;
	const int kBusterLevel1ChargeCount = 20;
	const int kBusterLevel2ChargeCount = 80;
	const int kBusterLevel3ChargeCount = 140;
	const int kBusterLevel4ChargeCount = 200;
}

PlayerCharacter::PlayerCharacter(ServerProxy* server)
    : AnimatedObject("animations/x.json", math::Vector2D(64.0, -16.0))
	, server_(server)
	, width_(8)
	, should_jump_(false)
	, holding_jump_(false)
	, should_dash_(false)
	, holding_dash_(false)
	, should_shoot_(false)
	, holding_shoot_(false)
	, dash_jump_(false)
	, shoot_anim_ticks_(kShootAnimationLength)
	, shoot_charge_ticks_(0)
    , show_dash_end_(false)
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

math::Vector2D PlayerCharacter::BulletOffsetForState() const {
	switch (state_) {
	
	case AnimationState::WALKING:
		return math::Vector2D(17, -21);

	case AnimationState::ON_AIR:
		return math::Vector2D(18, -27);

	case AnimationState::DASHING:
		return math::Vector2D(26, -14);
	
	case AnimationState::WALLSLIDING:
		return math::Vector2D(-21, -23);

	case AnimationState::WALLKICKING:
		return math::Vector2D(16, -27);

	//case AnimationState::WARPING:
	//case AnimationState::WARP_FINISH:
	//case AnimationState::STANDING:
	default:
		return math::Vector2D(8, -19);
	}
}

void PlayerCharacter::Shoot() {
    shoot_anim_ticks_++;

	auto charge_level = ChargeLevel(shoot_charge_ticks_);

	if ((should_shoot_ || (!holding_shoot_ && charge_level > 0))) {
        should_shoot_ = false;
        shoot_anim_ticks_ = 0;

		auto bullet_type = BulletTypeForLevel(charge_level);

		auto bullet_offset = BulletOffsetForState();
		bullet_offset.x *= direction_;

		int bullet_direction = direction_;
		if (state_ == AnimationState::WALLSLIDING)
			bullet_direction *= -1;

		server_->ShootBulletAt(position_ + bullet_offset,
							   bullet_type,
							   bullet_direction);
		
		shoot_charge_ticks_ = 0;
    }

	if (holding_shoot_)
		shoot_charge_ticks_++;
	else
		shoot_charge_ticks_ = 0;
}

void PlayerCharacter::Move() {
    switch (state_) {
    case AnimationState::DASHING:
        if (std::abs(input_x_axis_) > 0.2 && sgn(input_x_axis_) != direction_)
            state_ = AnimationState::WALKING;
        else
            break;
        // no break on purpose

    case AnimationState::STANDING:
    case AnimationState::WALKING:
    case AnimationState::ON_AIR:
    case AnimationState::WALLSLIDING:
        if (std::abs(input_x_axis_) > 0.2) {
            direction_ = sgn(input_x_axis_);
            velocity_.x = direction_ * (dash_jump_ ? kDashingSpeed : kWalkingSpeed);
            if (state_ == AnimationState::STANDING) {
                show_pre_walk_ = true;
                state_ = AnimationState::WALKING;
            }
        } else {
            velocity_.x = 0.0;
            if (state_ == AnimationState::WALKING)
                state_ = AnimationState::STANDING;
        }
        break;

    case AnimationState::WARPING:
    case AnimationState::WARP_FINISH:
    case AnimationState::WALLKICKING:
    default:
        break;
    }
}

void PlayerCharacter::Update(const FrameInput& input)
{
    if (position_.y > 1000) {
		position_.y = 0.0;
		auto& L = server_->collision().lua();
		L["world"]["update"](L["world"], collision_body_, position_.x - 4, position_.y - 16);
    }
    GetPlayerInput(input);
    Move();
    Dash();
	Jump();
    Shoot(); 
    ApplyGravity();
    ApplyVelocity();
	UpdateAnimation();
}

void PlayerCharacter::SetupCollision() {
	auto& L = server_->collision().lua();
	collision_body_ = L.create_table_with("name", "Player");
	L["world"]["add"](L["world"], collision_body_, position_.x - 4, position_.y - 16, width_, 16);
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
		if (show_dash_start_) {
			show_dash_start_ = false;
			server_->AddEffectAt(position_ + math::Vector2D(-direction_ * 32, 0.0),
								 Effect::Type::DASH_DUST,
								 direction_);
		}
		break;
	case AnimationState::WALKING:
		show_pre_walk_ = false;
        break;
    case AnimationState::STANDING:
        show_jump_recoil_ = false;
        show_dash_end_ = false;
	default:
		break;
    }
}

void PlayerCharacter::GetPlayerInput(const FrameInput& input) {
	should_jump_ = should_jump_ || (!holding_jump_ && input.holding_jump);
	should_dash_ = should_dash_ || (!holding_dash_ && input.holding_dash);
	should_shoot_ = should_shoot_ || (!holding_shoot_ && input.holding_shoot);

	input_x_axis_ = input.x_axis;
	holding_jump_= input.holding_jump;
	holding_dash_ = input.holding_dash;
	holding_shoot_ = input.holding_shoot;
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
	math::Vector2D offset(4, 16);
	auto new_pos = position_ + velocity_ - offset;

	auto& L = server_->collision().lua();
	std::tuple<double, double, sol::table, int> results =
		L["world"]["move"](L["world"], collision_body_, new_pos.x, new_pos.y);

	math::Vector2D actual_new_pos(std::get<0>(results), std::get<1>(results));

	position_ = actual_new_pos + offset;

	auto cols = std::get<2>(results);
	auto count = std::get<3>(results);

	for (auto i = 0; i < count; ++i) {
		sol::table collision = cols[i + 1];
		math::Vector2D normal(collision["normal"]["x"], collision["normal"]["y"]);

		if (normal.x != 0) {
			if (count > 1) {
				velocity_.x = 0.0;
				if (state_ == AnimationState::WALKING)
					state_ = AnimationState::STANDING;

			} else if (velocity_.y > 0.0) {
				if (state_ != AnimationState::WALLSLIDING) {
					show_wall_touch_ = true;
				}
				state_ = AnimationState::WALLSLIDING;
				dash_jump_ = false;
				velocity_.y = kWallSlidingSpeed;
			}
		}

		if (normal.y != 0) {
			velocity_.y = 0.0;
			if (normal.y < 0.0 && !on_ground())
				Land();
		}
	}

	if (count == 0) {
		switch (state_) {
		case AnimationState::WALLKICKING: break;
		case AnimationState::WARPING: break;
		case AnimationState::WARP_FINISH: break;
		case AnimationState::ON_AIR: break;
		
		case AnimationState::STANDING:
		case AnimationState::WALKING:
		case AnimationState::DASHING:
		case AnimationState::WALLSLIDING:
			state_ = AnimationState::ON_AIR;
		}
	}

	return;

	auto map = server_->map();

	// Movement in X
	auto xDirection = sgn(velocity_.x);
	auto cornerX = position_.x + xDirection * width_ / 2;
	auto cornerXwithVel = cornerX + velocity_.x;
	auto x_to_move = std::abs(velocity_.x);
	auto found_wall_x = false;

	std::vector<math::Vector2D> possible_tiles_x = {
		{cornerXwithVel, position_.y - map->tile_height()},
		{cornerXwithVel, position_.y},
	};

	for (const auto& pos : possible_tiles_x) {
		TileCoords tile_pos(map, pos.x, pos.y);
		if (is_solid(map, tile_pos)) {
			found_wall_x = true;
			auto tile_corner = tile_corner_x(map, tile_pos, -xDirection);
			auto tile_distance = std::abs(tile_corner - cornerX);
			x_to_move = std::min(tile_distance, x_to_move);
		}
	}

	if (found_wall_x) {
        if (on_ground()) {
            velocity_.x = 0.0;
            state_ = AnimationState::STANDING;

        } else if (velocity_.y > 0.0) {
		    if (state_ != AnimationState::WALLSLIDING) {
                show_wall_touch_ = true;
		    }
			state_ = AnimationState::WALLSLIDING;
            dash_jump_ = false;
			velocity_.y = kWallSlidingSpeed;
		}
	} else if (state_ == AnimationState::WALLSLIDING) {
		state_ = AnimationState::ON_AIR;
	}
	
	position_.x += xDirection * x_to_move;

	// Movement in Y
	auto yDirection = sgn(velocity_.y);
	auto cornerY = position_.y + 1.0/4.0;
	auto cornerYwithVel = cornerY + velocity_.y;

	auto y_to_move = std::abs(velocity_.y);
	auto found_wall_y = false;

	std::vector<math::Vector2D> possible_tiles_y = {
		{ position_.x - width_ / 2 + 1/4, cornerYwithVel },
		{ position_.x + width_ / 2 - 1/4, cornerYwithVel },
	};

	for (const auto& pos : possible_tiles_y) {
		TileCoords tile_pos(map, pos.x, pos.y);
		if (is_solid(map, tile_pos)) {
			found_wall_y = true;
			auto tile_corner = tile_corner_y(map, tile_pos, -yDirection);
			auto tile_distance = std::abs(tile_corner - cornerY);
			y_to_move = std::min(tile_distance, y_to_move);
		}
	}

	
	if (found_wall_y) {
		velocity_.y = 0.0;
		if (!on_ground())
			Land();
	} else if (on_ground()) {
		state_ = AnimationState::ON_AIR;
	}

	position_.y += yDirection * y_to_move;
}

void PlayerCharacter::Jump() {
	switch(state_)
	{
	case AnimationState::STANDING:
	case AnimationState::WALKING:
	case AnimationState::DASHING:
        if (should_jump_) {
            dash_jump_ = state_ == AnimationState::DASHING;
            state_ = AnimationState::ON_AIR;
            velocity_.y = -kJumpSpeed;
        }
		break;

	case AnimationState::WALLSLIDING:
        if (should_jump_) {
            state_ = AnimationState::WALLKICKING;
            show_wallkick_start_ = true;
            velocity_.y = 0.0;
        }
		break;

    case AnimationState::ON_AIR:
        if (velocity_.y < 0.0 && !holding_jump_)
            velocity_.y = 0.0;
        break;

	default:
		// Can't jump!
		break;
	}
    should_jump_ = false;
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
			show_dash_start_ = true;
		}
		break;

	case AnimationState::ON_AIR:
		// TODO: air dash!
		break;

	case AnimationState::DASHING:
		dash_ticks_++;
		if (!holding_dash_ || dash_ticks_ >= kDashLength) {
            show_dash_end_ = true;
            state_ = AnimationState::STANDING;
		} else if (dash_ticks_ % 4 == 1 && dash_ticks_ > 4) {
            server_->AddEffectAt(position_ + math::Vector2D(-direction_ * 10, -2.0), Effect::Type::DUST);
		}
		break;

	default:
		// Can't dash!
		break;
	}
	should_dash_ = false;
}

void PlayerCharacter::Land() {
    dash_jump_ = false;
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
        else if (show_dash_end_)
            ChangeAnimation("dashend");
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
		if (show_dash_start_)
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

int PlayerCharacter::ChargeLevel(int charge_time) {
	if (charge_time > kBusterLevel1ChargeCount) {
		return 1;
	}
	return 0;
}

Bullet::Type PlayerCharacter::BulletTypeForLevel(int level) {
	if (level >= 1)
		return Bullet::Type::X1_LV1;
	return Bullet::Type::X1_LV0;
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