#ifndef MAVERICK_BACKEND_BULLET_H_
#define MAVERICK_BACKEND_BULLET_H_

#include <memory>
#include <ugdk/input/events.h>
#include <ugdk/system/eventhandler.h>
#include <ugdk/action/observer.h>
#include <ugdk/action/spritetypes.h>

namespace backend {
class ServerProxy;

class Bullet
    : public ugdk::action::Observer
{
public:
	enum class Type {
		X1_LV0,
		X1_LV1,
	};
    explicit Bullet(const ugdk::math::Vector2D& position, Type type);

    void Update();
    void Tick() override;
    
    bool finished() const { return finished_; }
    const ugdk::math::Vector2D& position() const { return position_; }
    const ugdk::action::SpriteAnimationPlayer& player() const { return player_; }
	int direction() const { return direction_; }
	void set_direction(int direction) { direction_ = direction; }

	Type type() const { return type_; }

private:
    ugdk::math::Vector2D position_;
    ugdk::action::SpriteAnimationPlayer player_;
    bool finished_;
	Type type_;
	int direction_;
};

} // namespace backend

#endif // MAVERICK_BACKEND_BULLET_H_
