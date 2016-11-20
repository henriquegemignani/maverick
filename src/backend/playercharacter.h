#ifndef MAVERICK_BACKEND_PLAYERCHARACTER_H_
#define MAVERICK_BACKEND_PLAYERCHARACTER_H_

#include "backend/animatedobject.h"

#include <ugdk/action/observer.h>
#include <ugdk/action/spritetypes.h>
#include "backend/bullet.h"
#include "backend/effect.h"
#include "backend/frameinput.h"

#include <sol.hpp>

namespace backend {
class ServerProxy;

class PlayerCharacter 
    : public AnimatedObject
    , public ugdk::action::Observer
{
public:
    explicit PlayerCharacter(ServerProxy*);
	ugdk::math::Vector2D BulletOffsetForState() const;

	enum class AnimationState {
        WARPING,
        WARP_FINISH,
        STANDING,
        WALKING,
        ON_AIR,
        DASHING,
        WALLSLIDING,
		WALLKICKING,
    };

    void Update(const FrameInput&);
	void SetupCollision();

    void Tick() override;

    ugdk::action::SpriteAnimationPlayer& player() { return player_; }
	bool on_ground() const;
    int shoot_charge_ticks() const { return shoot_charge_ticks_; }

    static int ChargeLevel(int charge_time);

private:
    // Update body
    void GetPlayerInput(const FrameInput&);
    void Move();
    void Dash();
    void Jump();
    void Shoot();
    void ApplyGravity();
    void ApplyVelocity();
    void UpdateAnimation();

	static Bullet::Type BulletTypeForLevel(int level);

    // 
    void Land();

    void ChangeAnimation(const std::string&);

    ugdk::math::Vector2D velocity_;
    AnimationState state_;
	ServerProxy* server_;
	double width_;
	double input_x_axis_;
	bool should_jump_;
    bool holding_jump_;
	bool should_dash_;
	bool holding_dash_;
	bool should_shoot_;
	bool holding_shoot_;
    bool dash_jump_;
	int dash_ticks_;
	int shoot_anim_ticks_;
	int shoot_charge_ticks_;
    std::string current_animation_name_;

    bool show_dash_start_;
    bool show_dash_end_;
    bool show_pre_walk_;
	bool show_wall_touch_;
    bool show_wallkick_start_;
    bool show_jump_recoil_;
	sol::table collision_body_;
};

} // namespace backend

#endif // MAVERICK_BACKEND_PLAYERCHARACTER_H_
