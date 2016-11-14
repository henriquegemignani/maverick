#ifndef MAVERICK_BACKEND_FRAMEINPUT_H_
#define MAVERICK_BACKEND_FRAMEINPUT_H_

namespace backend {

struct FrameInput {
public:
	double x_axis;
	bool holding_jump;
	bool holding_dash;
	bool holding_shoot;
};

}

#endif // MAVERICK_BACKEND_FRAMEINPUT_H_