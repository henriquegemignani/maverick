#ifndef MAVERICK_BACKEND_PLAYERCHARACTER_H_
#define MAVERICK_BACKEND_PLAYERCHARACTER_H_

#include <memory>
#include <ugdk/input/events.h>
#include <ugdk/system/eventhandler.h>
#include <ugdk/action/observer.h>
#include <ugdk/action/spritetypes.h>

namespace backend {

class PlayerCharacter 
    : public ugdk::system::Listener<ugdk::input::JoystickDisconnectedEvent>
    , public ugdk::system::Listener<ugdk::input::JoystickAxisEvent>
    , public ugdk::system::Listener<ugdk::input::JoystickButtonPressedEvent>
    , public ugdk::system::Listener<ugdk::input::JoystickButtonReleasedEvent>
    , public ugdk::action::Observer
{
public:
    PlayerCharacter();

    enum class AnimationState {
        WARPING,
        WARP_FINISH,
        STANDING
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

private:
    ugdk::math::Vector2D position_, velocity_;
    bool on_ground_;
    double direction_;
    std::weak_ptr<ugdk::input::Joystick> current_joystick_;
    ugdk::action::SpriteAnimationPlayer player_;
    AnimationState state_;
};

} // namespace backend

#endif // MAVERICK_BACKEND_PLAYERCHARACTER_H_