#ifndef MAVERICK_FRONTEND_NATIVEBUILDERS_MAPRENDERER_H_
#define MAVERICK_FRONTEND_NATIVEBUILDERS_MAPRENDERER_H_

#include <tiled-reader/map.h>
#include <ugdk/graphic.h>
#include <ugdk/math.h>
#include <vector>
#include <functional>

namespace frontend {

class MapRenderer {
public:
	using DrawFunction = std::function<void(ugdk::graphic::Canvas& canvas, const ugdk::math::Frame& view)>;

    MapRenderer(const tiled::Map* map, 
                const DrawFunction& object_layer_drawfunction);

    void RenderLayers(ugdk::graphic::Canvas& canvas, const ugdk::math::Frame& view) const;

private:
	const tiled::Map* map_;
    std::vector<ugdk::graphic::GLTexture*> textures_;
	DrawFunction object_layer_drawfunction_;
};

} // namespace frontend

#endif // MAVERICK_FRONTEND_NATIVEBUILDERS_MAPRENDERER_H_
