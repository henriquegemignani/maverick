#ifndef MAVERICK_BACKEND_ANIMATEDOBJECT_H_
#define MAVERICK_BACKEND_ANIMATEDOBJECT_H_

#include "backend/atlasobject.h"
#include <ugdk/action/spritetypes.h>

namespace backend {

class AnimatedObject : public AtlasObject {
public:
	explicit AnimatedObject(const std::string& animations_name, const ugdk::math::Vector2D& position);
	virtual ~AnimatedObject() {}
    
	ugdk::action::SpriteAnimationFrame CurrentAnimationFrame() const override {
		return player_.current_animation_frame();
	}

protected:
	ugdk::action::SpriteAnimationPlayer player_;    	
};

} // namespace backend

#endif // MAVERICK_BACKEND_ANIMATEDOBJECT_H_
