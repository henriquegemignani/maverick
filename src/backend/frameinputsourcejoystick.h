#ifndef MAVERICK_BACKEND_FRAMEINPUTSOURCEJOYSTICK_H_
#define MAVERICK_BACKEND_FRAMEINPUTSOURCEJOYSTICK_H_

#include "backend/frameinputsource.h"
#include "ugdk/input/joystick.h"

namespace backend {

class FrameInputSourceJoystick : public FrameInputSource {
public:
	explicit FrameInputSourceJoystick(std::shared_ptr<ugdk::input::Joystick> joystick) : joystick_(joystick) {}
	virtual ~FrameInputSourceJoystick() = default;

	const static int kJumpJoystickKey = 11;
	const static int kDashJoystickKey = 12;
	const static int kShootJoystickKey = 13;

	FrameInput NextFrameInput() const override
	{
		return FrameInput {
			joystick_->GetAxisStatus(0).Percentage(),
			joystick_->IsDown(kJumpJoystickKey),
			joystick_->IsDown(kDashJoystickKey),
			joystick_->IsDown(kShootJoystickKey),
		};
	};

private:
	std::shared_ptr<ugdk::input::Joystick> joystick_;
};

}

#endif // MAVERICK_BACKEND_FRAMEINPUTSOURCEJOYSTICK_H_