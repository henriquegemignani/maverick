#ifndef MAVERICK_BACKEND_ATLASOBJECT_H_
#define MAVERICK_BACKEND_ATLASOBJECT_H_

#include <memory>
#include <ugdk/input/events.h>
#include <ugdk/system/eventhandler.h>
#include <ugdk/action/observer.h>
#include <ugdk/action/spritetypes.h>

namespace backend {

class AtlasObject {
public:
	explicit AtlasObject(const std::string& animations_name, const ugdk::math::Vector2D& position)
		: direction_(1)
		, position_(position)
		, animations_name_(animations_name) {}
	virtual ~AtlasObject() {}
    
	int direction() const { return direction_; }
	const ugdk::math::Vector2D& position() const { return position_; }

	virtual ugdk::action::SpriteAnimationFrame CurrentAnimationFrame() const = 0;
	const std::string& animations_name() const { return animations_name_; }

protected:
	int direction_;
	ugdk::math::Vector2D position_;

private:
	std::string animations_name_;
    	
};

} // namespace backend

#endif // MAVERICK_BACKEND_ATLASOBJECT_H_
