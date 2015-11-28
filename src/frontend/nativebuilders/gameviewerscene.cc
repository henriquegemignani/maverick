
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
	auto scene = std::make_unique<ugdk::action::Scene>();

    static ugdk::math::Vector2D camera;

    static PlayerCharacter player_character;

    static MapRenderer map_renderer(server_proxy->map(), [](ugdk::graphic::Canvas& canvas) {
        player_character.Render(canvas);
    });

	scene->set_render_function([=](ugdk::graphic::Canvas& canvas) {
        canvas.PushAndCompose(graphic::Geometry(-camera, math::Vector2D(2.0)));
        map_renderer.RenderLayers(canvas);
        canvas.PopGeometry();
	});

    scene->event_handler().AddListener< ugdk::input::KeyPressedEvent >([](const ugdk::input::KeyPressedEvent& ev) {
        if (ev.scancode == ugdk::input::Scancode::T) {
            player_character.GoToNextFrame();
        }
    });


    scene->event_handler().AddListener<ugdk::input::JoystickConnectedEvent>([=](const ugdk::input::JoystickConnectedEvent& ev) {
        player_character.HandleNewJoystick(ev.joystick.lock());
    });
    auto joysticks = ugdk::input::manager()->CurrentJoysticks();
    if (!joysticks.empty()) {
        player_character.HandleNewJoystick(joysticks.front());
    }

    scene->AddTask([](double dt) {
        player_character.Update(dt);
    });

	return std::move(scene);
}

} // namespace nativebuilders
} // namespace frontend
