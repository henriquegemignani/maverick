#ifndef MAVERICK_FRONTEND_NATIVEBUILDERS_MAPRENDERER_H_
#define MAVERICK_FRONTEND_NATIVEBUILDERS_MAPRENDERER_H_

#include <tiled-reader/map.h>
#include <ugdk/graphic.h>

namespace frontend {

class MapRenderer {
public:
    MapRenderer(const tiled::Map* map)
		: map_(map) {}

    void RenderLayers(ugdk::graphic::Canvas& canvas) const;

private:
	const tiled::Map* map_;
};

} // namespace frontend

#endif // MAVERICK_FRONTEND_NATIVEBUILDERS_MAPRENDERER_H_
