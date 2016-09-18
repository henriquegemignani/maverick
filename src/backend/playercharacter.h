#ifndef MAVERICK_BACKEND_PLAYERCHARACTER_H_
#define MAVERICK_BACKEND_PLAYERCHARACTER_H_

#include <memory>
#include <ugdk/input/events.h>
#include <ugdk/system/eventhandler.h>
#include <ugdk/action/observer.h>
#include <ugdk/action/spritetypes.h>

namespace backend {
class ServerProxy;

class PlayerCharacter 
    : public ugdk::system::Listener<ugdk::input::JoystickDisconnectedEvent>
    , public ugdk::system::Listener<ugdk::input::JoystickAxisEvent>
    , public ugdk::system::Listener<ugdk::input::JoystickButtonPressedEvent>
    , public ugdk::system::Listener<ugdk::input::JoystickButtonReleasedEvent>
    , public ugdk::action::Observer
{
public:
    explicit PlayerCharacter(ServerProxy*);

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

    void Update(double dt);

    void HandleNewJoystick(std::shared_ptr<ugdk::input::Joystick>);
    void Handle(const ugdk::input::JoystickDisconnectedEvent& ev) override;
    void Handle(const ugdk::input::JoystickAxisEvent& ev) override;
    void Handle(const ugdk::input::JoystickButtonPressedEvent& ev) override;
    void Handle(const ugdk::input::JoystickButtonReleasedEvent& ev) override;
    void Tick() override;

    double direction() const { return direction_; }
    const ugdk::math::Vector2D& position() const { return position_; }
    ugdk::action::SpriteAnimationPlayer& player() { return player_; }
	bool on_ground() const;
	bool IsAcceptingMovementInput() const;

private:
    void GetPlayerInput();
    void ApplyGravity();
    void ApplyVelocity();
	void UpdateAnimation();
	void Jump();
	void Dash();
	void Land();

    void ChangeAnimation(const std::string&);

    ugdk::math::Vector2D position_, velocity_;
    int direction_;
    std::weak_ptr<ugdk::input::Joystick> current_joystick_;
    ugdk::action::SpriteAnimationPlayer player_;
    AnimationState state_;
	ServerProxy* server_;
	double width_;
	double input_x_axis_;
	bool should_jump_;
    bool holding_jump_;
	bool should_dash_;
	bool holding_dash_;
	int dash_ticks_;
	int shoot_anim_ticks_;
	bool should_shoot_;
    std::string current_animation_name_;

    bool dash_start_;
    bool show_pre_walk_;
	bool show_wall_touch_;
    bool show_wallkick_start_;
    bool show_jump_recoil_;
};

} // namespace backend

#endif // MAVERICK_BACKEND_PLAYERCHARACTER_H_
