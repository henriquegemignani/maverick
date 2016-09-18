#ifndef MAVERICK_BACKEND_PLAYERCHARACTER_H_
#define MAVERICK_BACKEND_PLAYERCHARACTER_H_

#include "backend/animatedobject.h"
#include <memory>
#include <ugdk/input/events.h>
#include <ugdk/system/eventhandler.h>
#include <ugdk/action/observer.h>
#include <ugdk/action/spritetypes.h>
#include "bullet.h"

namespace backend {
class ServerProxy;

class PlayerCharacter 
    : public AnimatedObject
	, public ugdk::system::Listener<ugdk::input::JoystickDisconnectedEvent>
    , public ugdk::system::Listener<ugdk::input::JoystickAxisEvent>
    , public ugdk::system::Listener<ugdk::input::JoystickButtonPressedEvent>
    , public ugdk::system::Listener<ugdk::input::JoystickButtonReleasedEvent>
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

    void Update();

    void HandleNewJoystick(std::shared_ptr<ugdk::input::Joystick>);
    void Handle(const ugdk::input::JoystickDisconnectedEvent& ev) override;
    void Handle(const ugdk::input::JoystickAxisEvent& ev) override;
    void Handle(const ugdk::input::JoystickButtonPressedEvent& ev) override;
    void Handle(const ugdk::input::JoystickButtonReleasedEvent& ev) override;
    void Tick() override;

    ugdk::action::SpriteAnimationPlayer& player() { return player_; }
	bool on_ground() const;

private:
    // Update body
    void GetPlayerInput();
    void Move();
    void Dash();
    void Jump();
    void Shoot();
    void ApplyGravity();
    void ApplyVelocity();
    void UpdateAnimation();

	static int ChargeLevel(int charge_time);
	static Bullet::Type BulletTypeForLevel(int level);

    // 
    void Land();

    void ChangeAnimation(const std::string&);

    ugdk::math::Vector2D velocity_;
    std::weak_ptr<ugdk::input::Joystick> current_joystick_;
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
};

} // namespace backend

#endif // MAVERICK_BACKEND_PLAYERCHARACTER_H_
