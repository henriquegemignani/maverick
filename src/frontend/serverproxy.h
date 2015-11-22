#ifndef MAVERICK_FRONTEND_SERVERPROXY_H_
#define MAVERICK_FRONTEND_SERVERPROXY_H_

#include <memory>

namespace frontend {

struct ServerProxyImpl;

class ServerProxy {
public:
	~ServerProxy();
	static ServerProxy* reference();

private:
	ServerProxy();
	std::unique_ptr<ServerProxyImpl> impl_;

};

} // namespace frontend

#endif // MAVERICK_FRONTEND_SERVERPROXY_H_
