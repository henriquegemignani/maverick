
#include "frontend/nativebuilders/maprenderer.h"

#include <ugdk/action/scene.h>
#include <ugdk/graphic/canvas.h>
#include <ugdk/graphic/module.h>
#include <ugdk/graphic/primitive.h>
#include <ugdk/graphic/textureatlas.h>
#include <ugdk/graphic/primitivesetup.h>
#include <ugdk/graphic/opengl.h>
#include <ugdk/graphic/sprite.h>
#include <ugdk/resource/module.h>
#include <ugdk/math/frame.h>
#include <ugdk/ui/drawable/texturedrectangle.h>
#include <tiled-reader/stdiofileloader.h>

namespace frontend {

using namespace ugdk;

namespace {
    struct VertexXYUV {
        float x, y, u, v;
    };

    void PopulateVertexDataWithTileInfo(graphic::VertexData& data, const tiled::TileInfo& info, int col, int row) {
        ugdk::graphic::VertexData::Mapper mapper(data, false);

        VertexXYUV* v1 = mapper.Get<VertexXYUV>(0);
        v1->x = col * info.tile_width;
        v1->y = row * info.tile_height;
        v1->u = info.p1_u;
        v1->v = info.p1_v;

        VertexXYUV* v2 = mapper.Get<VertexXYUV>(1);
        v2->x = v1->x;
        v2->y = v1->y + info.tile_height;
        v2->u = info.p1_u;
        v2->v = info.p2_v;

        VertexXYUV* v3 = mapper.Get<VertexXYUV>(2);
        v3->x = v1->x + info.tile_width;
        v3->y = v1->y;
        v3->u = info.p2_u;
        v3->v = info.p1_v;

        VertexXYUV* v4 = mapper.Get<VertexXYUV>(3);
        v4->x = v3->x;
        v4->y = v2->y;
        v4->u = info.p2_u;
        v4->v = info.p2_v;
    }
}

MapRenderer::MapRenderer(const tiled::Map* map,
                         const DrawFunction& object_layer_drawfunction)
    : map_(map)
    , textures_(map->tileset_count(), nullptr)
    , object_layer_drawfunction_(object_layer_drawfunction)
{
    std::string&& dirname = tiled::StdioFileLoader().GetDirnameOfPath(map_->filepath());
    for (int id = 0; id < textures_.size(); ++id) {
        textures_[id] = ugdk::resource::GetTextureFromFile(dirname + "/" + map_->tileset(id)->asset_name());
    }
}

void MapRenderer::RenderLayers(ugdk::graphic::Canvas & canvas, const ugdk::math::Frame& view) const
{
    std::vector<ugdk::graphic::TextureUnit> texture_units;
    texture_units.reserve(textures_.size());
    for (std::size_t id = 0; id < textures_.size(); ++id) {
        texture_units.emplace_back(ugdk::graphic::manager()->ReserveTextureUnit(textures_[id]));
    }

    ugdk::graphic::VertexData data(4, sizeof(VertexXYUV), true, true);

	int first_col = static_cast<int>(std::floor(view.left() / map_->tile_width()));
	int last_col = static_cast<int>(std::ceil((view.right()) / map_->tile_width()));
	int first_row = static_cast<int>(std::floor(view.top() / map_->tile_height()));
	int last_row = static_cast<int>(std::ceil((view.bottom()) / map_->tile_height()));

    auto render_layer = [&](const tiled::Layer& layer) {
		int target_height = std::min(layer.height() - 1, last_row);
		int target_width = std::min(layer.width() - 1, last_col);
        for (int row = first_row; row <= target_height; ++row) {
            for (int col = first_col; col <= target_width; ++col) {
                auto tile = layer.tile_at(col, row);
                if (tile.gid == 0) continue;
                tiled::TileInfo info = map_->tileinfo_for(tile);

                canvas.SendUniform("drawable_texture", texture_units[info.tileset->id()]);
                PopulateVertexDataWithTileInfo(data, info, col, row);

                canvas.SendVertexData(data, ugdk::graphic::VertexType::VERTEX, 0, 2);
                canvas.SendVertexData(data, ugdk::graphic::VertexType::TEXTURE, 2 * sizeof(float), 2);
                canvas.DrawArrays(ugdk::graphic::DrawMode::TRIANGLE_STRIP(), 0, 4);
            }
        }
    };

    for (const auto& layer : map_->layers()) {
        switch (layer.type()) {
        case tiled::Layer::Type::TileLayer:
            render_layer(layer);
            break;
        case tiled::Layer::Type::ObjectGroup:
            object_layer_drawfunction_(canvas, view);
            break;
        case tiled::Layer::Type::ImageLayer:
            throw system::BaseException("Layer::Type::ImageLayer is unsupported");
            break;
        }
    }
}

} // namespace frontend