#ifndef MAVERICK_FRONTEND_SERVERPROXY_H_
#define MAVERICK_FRONTEND_SERVERPROXY_H_

#include <memory>
#include <tiled-reader/map.h>

namespace backend {

struct ServerProxyImpl;

class ServerProxy {
public:
	~ServerProxy();
	static ServerProxy* reference();
	const tiled::Map* map() const;

private:
	ServerProxy();
	std::unique_ptr<ServerProxyImpl> impl_;

};

} // namespace frontend

#endif // MAVERICK_FRONTEND_SERVERPROXY_H_
