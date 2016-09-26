
#include "backend/collision.h"

#include <ugdk/filesystem/module.h>
#include <ugdk/filesystem/file.h>

namespace backend {

Collision::Collision() {
	lua_.open_libraries();

	auto file = ugdk::filesystem::manager()->OpenFile("bump.lua");
	lua_.require_script("bump", file->GetContents());
	lua_.script("world = bump.newWorld(8)");
}

Collision::~Collision() {
}

} // namespace backend