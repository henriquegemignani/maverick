
#include "frontend/nativebuilders.h"

#include "frontend/serverproxy.h"
#include <ugdk/action/scene.h>
#include <ugdk/graphic/canvas.h>
#include <ugdk/graphic/module.h>
#include <ugdk/graphic/primitive.h>
#include <ugdk/graphic/textureatlas.h>
#include <ugdk/graphic/primitivesetup.h>
#include <ugdk/graphic/sprite.h>
#include <ugdk/resource/module.h>
#include <ugdk/ui/drawable/texturedrectangle.h>
#include <ugdk/input/events.h>
#include <ugdk/input/module.h>
#include <tiled-reader/stdiofileloader.h>

namespace frontend {

namespace nativebuilders {

namespace {
	struct VertexXYUV {
		float x, y, u, v;
	};
}

std::unique_ptr<ugdk::action::Scene> GameViewerScene() {
	ServerProxy* server_proxy = ServerProxy::reference();
	auto map = server_proxy->map();
	auto scene = std::make_unique<ugdk::action::Scene>();

	static std::unordered_map<std::string, ugdk::graphic::GLTexture*> textures_;
	auto texture_getter = [=](const std::string& path) {
		auto& t = textures_[path];
		if (t) {
			return t;
		}
		return t = ugdk::resource::GetTextureFromFile(tiled::StdioFileLoader().GetDirnameOfPath(map->filepath()) + "/" + path);
	};

	static ugdk::graphic::Primitive player_primitive(nullptr, nullptr);
	ugdk::graphic::PrimitiveSetup::Sprite::Prepare(player_primitive, ugdk::resource::GetTextureAtlasFromFile("repo"));

	static auto controller = dynamic_cast<ugdk::graphic::PrimitiveControllerSprite*>(player_primitive.controller().get());
    static size_t current_frame = 0;
    controller->ChangeToAtlasFrame(current_frame++);
	//static auto set = ugdk::resource::GetSpriteAnimationTableFromFile("repo.json");

	scene->set_render_function([=](ugdk::graphic::Canvas& canvas) {
		ugdk::graphic::TextureUnit unit = ugdk::graphic::manager()->ReserveTextureUnit(nullptr);
		ugdk::graphic::TextureUnit sprite_unit = ugdk::graphic::manager()->ReserveTextureUnit(player_primitive.texture());
		ugdk::graphic::VertexData data(4, sizeof(VertexXYUV), true, true);

		auto render_layer = [&](const tiled::Layer& layer) {
			for (int row = 0; row < layer.height(); ++row) {
				for (int col = 0; col < layer.width(); ++col) {
					auto tile = layer.tile_at(col, row);
					if (tile.gid == 0) continue;
					tiled::TileInfo info = map->tileinfo_for(tile);
					unit.BindTexture(texture_getter(info.asset_name));

					{
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

					canvas.SendVertexData(data, ugdk::graphic::VertexType::VERTEX, 0, 2);
					canvas.SendVertexData(data, ugdk::graphic::VertexType::TEXTURE, 2 * sizeof(float), 2);
					canvas.DrawArrays(ugdk::graphic::DrawMode::TRIANGLE_STRIP(), 0, 4);
				}
			}
		};

		canvas.SendUniform("drawable_texture", unit);
		render_layer(map->layers()[0]);

		canvas.SendUniform("drawable_texture", sprite_unit);
		player_primitive.drawfunction()(player_primitive, canvas);

		canvas.SendUniform("drawable_texture", unit);
		render_layer(map->layers()[1]);
	});

    scene->event_handler().AddListener< ugdk::input::KeyPressedEvent >([](const ugdk::input::KeyPressedEvent& ev) {
        if (ev.scancode == ugdk::input::Scancode::T) {
            controller->ChangeToAtlasFrame(current_frame++);
        }
    });

	return std::move(scene);
}

} // namespace nativebuilders
} // namespace frontend
