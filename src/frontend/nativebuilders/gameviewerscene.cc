
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

#include "frontend/nativebuilders/playercharacter.h"
#include "frontend/nativebuilders/maprenderer.h"

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


	static ugdk::graphic::Primitive player_primitive(nullptr, nullptr);
	ugdk::graphic::PrimitiveSetup::Sprite::Prepare(player_primitive, ugdk::resource::GetTextureAtlasFromFile("repo"));

	static auto controller = dynamic_cast<ugdk::graphic::PrimitiveControllerSprite*>(player_primitive.controller().get());
    static size_t current_frame = 0;
    controller->ChangeToAtlasFrame(current_frame++);
	//static auto set = ugdk::resource::GetSpriteAnimationTableFromFile("repo.json");

    static ugdk::math::Vector2D camera, player_position, player_velocity;
    static MapRenderer map_renderer(map);

	scene->set_render_function([=](ugdk::graphic::Canvas& canvas) {
        canvas.PushAndCompose(graphic::Geometry(-camera, math::Vector2D(2.0)));
        map_renderer.RenderLayers(canvas);

        ugdk::graphic::TextureUnit sprite_unit = ugdk::graphic::manager()->ReserveTextureUnit(player_primitive.texture());
        canvas.PushAndCompose(graphic::Geometry(player_position));
		canvas.SendUniform("drawable_texture", sprite_unit);
		player_primitive.drawfunction()(player_primitive, canvas);
        canvas.PopGeometry();

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
