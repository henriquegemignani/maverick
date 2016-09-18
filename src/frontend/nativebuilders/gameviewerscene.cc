
#include "frontend/nativebuilders.h"

#include <ugdk/action/scene.h>
#include <ugdk/graphic/canvas.h>
#include <ugdk/input/events.h>
#include <ugdk/input/module.h>
#include <ugdk/input/joystick.h>
#include <ugdk/math/frame.h>

#include "backend/serverproxy.h"
#include "backend/playercharacter.h"
#include "frontend/nativebuilders/maprenderer.h"
#include "objectviewer.h"

namespace frontend {

namespace nativebuilders {

namespace {
	struct VertexXYUV {
		float x, y, u, v;
	};
}

using namespace ugdk;

std::unique_ptr<ugdk::action::Scene> GameViewerScene() {
	auto server_proxy = backend::ServerProxy::reference();
	auto scene = std::make_unique<ugdk::action::Scene>();

    static ugdk::math::Vector2D camera;

    static ObjectViewer object_viewer(server_proxy);
    static MapRenderer map_renderer(server_proxy->map(), [](ugdk::graphic::Canvas& canvas, const ugdk::math::Frame& view) {
        object_viewer.Render(canvas);
    });

	scene->set_render_function([=](ugdk::graphic::Canvas& canvas) {
        canvas.PushAndCompose(math::Geometry(-camera, math::Vector2D(2.0)));
		auto actual_size = canvas.size() * 0.5;
		auto left_corner = camera * 0.5;
        map_renderer.RenderLayers(canvas, ugdk::math::Frame(left_corner.x, left_corner.y, left_corner.x + actual_size.x, left_corner.y + actual_size.y));
        canvas.PopGeometry();
	});

    scene->event_handler().AddListener<ugdk::input::JoystickConnectedEvent>([=](const ugdk::input::JoystickConnectedEvent& ev) {
        server_proxy->player_character().HandleNewJoystick(ev.joystick.lock());
    });
    auto joysticks = ugdk::input::manager()->CurrentJoysticks();
    if (!joysticks.empty()) {
        server_proxy->player_character().HandleNewJoystick(joysticks.front());
    }

    scene->AddTask([](double dt) {
        auto server_proxy = backend::ServerProxy::reference();
        server_proxy->Tick();
		camera.x = std::floor(server_proxy->player_character().position().x * 2 - 400.0);
    });

	return std::move(scene);
}

} // namespace nativebuilders
} // namespace frontend
