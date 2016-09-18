#ifndef MAVERICK_FRONTEND_SERVERPROXY_H_
#define MAVERICK_FRONTEND_SERVERPROXY_H_

#include <memory>
#include <tiled-reader/map.h>

namespace backend {
class PlayerCharacter;
struct ServerProxyImpl;

class ServerProxy {
public:
	~ServerProxy();
	static ServerProxy* reference();
	
    void Tick();
    const tiled::Map* map() const;
    PlayerCharacter& player_character();

private:
	ServerProxy();
	std::unique_ptr<ServerProxyImpl> impl_;
};

} // namespace frontend

#endif // MAVERICK_FRONTEND_SERVERPROXY_H_
