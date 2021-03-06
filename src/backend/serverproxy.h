#ifndef MAVERICK_FRONTEND_SERVERPROXY_H_
#define MAVERICK_FRONTEND_SERVERPROXY_H_

#include <memory>
#include <tiled-reader/map.h>
#include <ugdk/math.h>
#include "effect.h"
#include "bullet.h"


namespace backend {
class PlayerCharacter;
struct ServerProxyImpl;
class Collision;

class ServerProxy {
public:
	~ServerProxy();
	void HandleNewJoystick(const std::shared_ptr<ugdk::input::Joystick>& joystick);
	static ServerProxy* reference();
	
    void Tick();
    const tiled::Map* map() const;
    PlayerCharacter& player_character();
    const std::list<Effect>& effects() const;
	const std::list<Bullet>& bullets() const;
	Collision& collision();

	void AddEffectAt(const ugdk::math::Vector2D& position, Effect::Type);
    void AddEffectAt(const ugdk::math::Vector2D& position, Effect::Type, int direction);
	void ShootBulletAt(const ugdk::math::Vector2D& position, Bullet::Type, int direction);

private:
	ServerProxy();
	std::unique_ptr<ServerProxyImpl> impl_;
};

} // namespace frontend

#endif // MAVERICK_FRONTEND_SERVERPROXY_H_
