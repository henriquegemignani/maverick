#ifndef MAVERICK_SERVER_PLATFORMINGCORE_H_
#define MAVERICK_SERVER_PLATFORMINGCORE_H_

#include <tiled-reader/map.h>
#include <memory>

namespace server {

class PlatformingCore {
public:
	PlatformingCore(std::unique_ptr<tiled::Map> map)
		: map_(std::move(map)) {}
	
	const tiled::Map* map() const { return map_.get(); }

private:
	std::unique_ptr<tiled::Map> map_;
};

} // namespace server

#endif // MAVERICK_SERVER_PLATFORMINGCORE_H_
