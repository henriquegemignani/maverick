#ifndef MAVERICK_BACKEND_EFFECT_H_
#define MAVERICK_BACKEND_EFFECT_H_

#include "backend/animatedobject.h"

#include <ugdk/action/observer.h>
#include <ugdk/action/spritetypes.h>

namespace backend {

class Effect 
	: public AnimatedObject
	, public ugdk::action::Observer
{
public:
	enum class Type {
		DUST,
		DASH_DUST,
	};
    explicit Effect(const ugdk::math::Vector2D& position, Type type);

    void Update();
    void Tick() override;
    
    bool finished() const { return finished_; }
	void set_direction(int direction) { direction_ = direction; }
	Type type() const { return type_; }

private:
    bool finished_;
	Type type_;
};

} // namespace backend

#endif // MAVERICK_BACKEND_EFFECT_H_
