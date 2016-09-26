#ifndef MAVERICK_FRONTEND_COLLISION_H_
#define MAVERICK_FRONTEND_COLLISION_H_

#include <sol.hpp>

namespace backend {

class Collision {
public:
	Collision();
	~Collision();

	sol::state& lua() { return lua_; };

private:
	sol::state lua_;
};

} // namespace backend

#endif // MAVERICK_FRONTEND_COLLISION_H_
