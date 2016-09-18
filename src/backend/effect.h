#ifndef MAVERICK_BACKEND_EFFECT_H_
#define MAVERICK_BACKEND_EFFECT_H_

#include <memory>
#include <ugdk/input/events.h>
#include <ugdk/system/eventhandler.h>
#include <ugdk/action/observer.h>
#include <ugdk/action/spritetypes.h>

namespace backend {
class ServerProxy;

class Effect 
    : public ugdk::action::Observer
{
public:
    explicit Effect(const ugdk::math::Vector2D& position, const std::string& effect_name);

    void Update(double dt);
    void Tick() override;
    
    bool finished() const { return finished_; }
    const ugdk::math::Vector2D& position() const { return position_; }
    ugdk::action::SpriteAnimationPlayer& player() { return player_; }

private:
    ugdk::math::Vector2D position_;
    ugdk::action::SpriteAnimationPlayer player_;
    bool finished_;
};

} // namespace backend

#endif // MAVERICK_BACKEND_EFFECT_H_
