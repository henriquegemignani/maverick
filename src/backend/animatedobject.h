#ifndef MAVERICK_BACKEND_ANIMATEDOBJECT_H_
#define MAVERICK_BACKEND_ANIMATEDOBJECT_H_

#include <memory>
#include <ugdk/input/events.h>
#include <ugdk/system/eventhandler.h>
#include <ugdk/action/observer.h>
#include <ugdk/action/spritetypes.h>

namespace backend {

class AnimatedObject {
public:
	explicit AnimatedObject(const std::string& animations_name, const ugdk::math::Vector2D& position);
	virtual ~AnimatedObject() {}
    
	int direction() const { return direction_; }
	const ugdk::math::Vector2D& position() const { return position_; }
	const ugdk::action::SpriteAnimationPlayer& player() const { return player_; }
	const std::string& animations_name() const { return animations_name_; }

protected:
	int direction_;
	ugdk::math::Vector2D position_;
	ugdk::action::SpriteAnimationPlayer player_;

private:
	std::string animations_name_;
    	
};

} // namespace backend

#endif // MAVERICK_BACKEND_ANIMATEDOBJECT_H_
