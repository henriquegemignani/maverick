#ifndef MAVERICK_BACKEND_BULLET_H_
#define MAVERICK_BACKEND_BULLET_H_

#include "backend/animatedobject.h"

#include <ugdk/action/observer.h>
#include <ugdk/action/spritetypes.h>

namespace backend {

class Bullet
    : public AnimatedObject
	, public ugdk::action::Observer
{
public:
	enum class Type {
		X1_LV0,
		X1_LV1,
	};
	enum class State {
		START,
		PRELOOP,
		LOOP,
		HIT
	};
    explicit Bullet(const ugdk::math::Vector2D& position, Type type);

    void Update();
    void Tick() override;
    
    bool finished() const { return finished_; }
	void set_direction(int direction) { direction_ = direction; }
	Type type() const { return type_; }

private:
    bool finished_;
	Type type_;
	int time_alive_;
	State state_;
	double player_offset_x_;
	bool first_update_;
};

} // namespace backend

#endif // MAVERICK_BACKEND_BULLET_H_
