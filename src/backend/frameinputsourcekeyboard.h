#ifndef MAVERICK_BACKEND_FRAMEINPUTSOURCEKEYBOARD_H_
#define MAVERICK_BACKEND_FRAMEINPUTSOURCEKEYBOARD_H_

#include "backend/frameinputsource.h"
#include "ugdk/input/module.h"

namespace backend {

class FrameInputSourceKeyboard : public FrameInputSource {
public:
	virtual ~FrameInputSourceKeyboard() = default;

	FrameInput NextFrameInput() const override {
		using namespace ugdk;
		const auto& keyboard = input::manager()->keyboard();
		double input_x_axis;
		if (keyboard.IsDown(input::Scancode::D) || keyboard.IsDown(input::Scancode::RIGHT)) {
			input_x_axis = 1.0;
		} else if (keyboard.IsDown(input::Scancode::A) || keyboard.IsDown(input::Scancode::LEFT)) {
			input_x_axis = -1.0;
		} else {
			input_x_axis = 0.0;
		}
		return FrameInput{
			input_x_axis,
			keyboard.IsDown(input::Scancode::SPACE),
			keyboard.IsDown(input::Scancode::LSHIFT),
			keyboard.IsDown(input::Scancode::Z),
		};
	};
};

}

#endif // MAVERICK_BACKEND_FRAMEINPUTSOURCEKEYBOARD_H_