#ifndef MAVERICK_FRONTEND_NATIVEBUILDERS_PLAYERCHARACTER_H_
#define MAVERICK_FRONTEND_NATIVEBUILDERS_PLAYERCHARACTER_H_

#include <memory>
#include <ugdk/graphic/sprite.h>
#include <ugdk/graphic/primitive.h>
#include <ugdk/input/events.h>
#include <ugdk/system/eventhandler.h>
#include <ugdk/action/observer.h>

namespace frontend {

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
    void Render(ugdk::graphic::Canvas&) const;

    void HandleNewJoystick(std::shared_ptr<ugdk::input::Joystick>);
    void Handle(const ugdk::input::JoystickDisconnectedEvent& ev) override;
    void Handle(const ugdk::input::JoystickAxisEvent& ev) override;
    void Handle(const ugdk::input::JoystickButtonPressedEvent& ev) override;
    void Handle(const ugdk::input::JoystickButtonReleasedEvent& ev) override;
    void Tick() override;

private:
    ugdk::graphic::Primitive primitive_;
    ugdk::graphic::PrimitiveControllerSprite* controller_;
    ugdk::math::Vector2D position_, velocity_;
    bool on_ground_;
    double direction_;
    std::weak_ptr<ugdk::input::Joystick> current_joystick_;
    ugdk::graphic::SpriteAnimationPlayer player_;
    AnimationState state_;
};

} // namespace frontend

#endif // MAVERICK_FRONTEND_NATIVEBUILDERS_PLAYERCHARACTER_H_
