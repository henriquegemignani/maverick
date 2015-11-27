
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
#include <ugdk/input/joystick.h>
#include <tiled-reader/stdiofileloader.h>

namespace frontend {

namespace nativebuilders {

namespace {
	struct VertexXYUV {
		float x, y, u, v;
	};
}

using namespace ugdk;



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

    static ugdk::math::Vector2D camera, player_position, player_velocity;

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

        canvas.PushAndCompose(graphic::Geometry(-camera, math::Vector2D(2.0)));

		canvas.SendUniform("drawable_texture", unit);
		render_layer(map->layers()[0]);

        canvas.PushAndCompose(graphic::Geometry(player_position));
		canvas.SendUniform("drawable_texture", sprite_unit);
		player_primitive.drawfunction()(player_primitive, canvas);
        canvas.PopGeometry();

		canvas.SendUniform("drawable_texture", unit);
		render_layer(map->layers()[1]);

        canvas.PopGeometry();
	});

    scene->event_handler().AddListener< ugdk::input::KeyPressedEvent >([](const ugdk::input::KeyPressedEvent& ev) {
        if (ev.scancode == ugdk::input::Scancode::T) {
            controller->ChangeToAtlasFrame(current_frame++);
        }
    });

    static bool on_ground = true;

    // 
    auto axis_func = [](const input::JoystickAxisEvent& ev) {        
    };
    auto button_press_func = [](const input::JoystickButtonPressedEvent& ev) {
        if (ev.button == 11) {
            if (on_ground) {
                on_ground = false;
                player_velocity.y = -256.0;
            }
        }
    };
    auto button_release_func = [](const input::JoystickButtonReleasedEvent& ev) {
    };

    auto new_joystick = [=](std::shared_ptr<input::Joystick> joy) {
        joy->event_handler().AddListener< input::JoystickAxisEvent >(axis_func);
        joy->event_handler().AddListener< input::JoystickButtonPressedEvent >(button_press_func);
        joy->event_handler().AddListener< input::JoystickButtonReleasedEvent >(button_release_func);
    };

    scene->event_handler().AddListener<ugdk::input::JoystickConnectedEvent>([=](const ugdk::input::JoystickConnectedEvent& ev) {
        new_joystick(ev.joystick.lock());
    });
    auto joysticks = ugdk::input::manager()->CurrentJoysticks();
    if (!joysticks.empty()) {
        new_joystick(joysticks.front());
    }

    scene->AddTask([](double dt) {
        auto joysticks = ugdk::input::manager()->CurrentJoysticks();
        if (!joysticks.empty()) {
            auto joystick = joysticks.front();
            double x_axis = joystick->GetAxisStatus(0).Percentage();
            if (abs(x_axis) > 0.2)
                player_position.x += x_axis * 60 * dt;
        }
        player_position += player_velocity * dt;
        player_velocity.y += 512 * dt;
        player_velocity.y = std::min(player_velocity.y, 512.0);
        if (player_position.y > 96.0) {
            player_position.y = 96.0;
            player_velocity.y = 0.0;
            on_ground = true;
        }
    });

	return std::move(scene);
}

} // namespace nativebuilders
} // namespace frontend
