
#include "frontend/nativebuilders/objectviewer.h"

#include "backend/playercharacter.h"

#include <ugdk/action/scene.h>
#include <ugdk/graphic/canvas.h>
#include <ugdk/graphic/module.h>
#include <ugdk/graphic/primitive.h>
#include <ugdk/graphic/textureatlas.h>
#include <ugdk/graphic/primitivesetup.h>
#include <ugdk/resource/module.h>
#include <ugdk/ui/drawable/texturedrectangle.h>
#include <ugdk/graphic/immediate.h>

namespace frontend {
  
using namespace ugdk;

namespace {

graphic::TextureAtlas* x_atlas = nullptr;
graphic::TextureAtlas* dust_atlas = nullptr;
graphic::TextureAtlas* dash_dust_atlas = nullptr;
graphic::TextureAtlas* buster_atlas = nullptr;
graphic::TextureAtlas* buster_charge_atlas = nullptr;

void populate_atlas() {
	if (x_atlas) return;

	x_atlas = resource::GetTextureAtlasFromFile("spritesheets/x");
	dust_atlas = resource::GetTextureAtlasFromFile("spritesheets/dust");
	dash_dust_atlas = resource::GetTextureAtlasFromFile("spritesheets/dash_dust");
	buster_atlas = resource::GetTextureAtlasFromFile("spritesheets/buster");
	buster_charge_atlas = resource::GetTextureAtlasFromFile("spritesheets/buster-charge");
}

std::tuple<graphic::TextureAtlas*, math::Vector2D>
get_data_for(const std::string& animtions_name) {

	if (animtions_name == "animations/x.json") {
		return std::make_tuple(x_atlas, math::Vector2D(-32, -56.0));
		
	} else if (animtions_name == "animations/dust.json") {
		return std::make_tuple(dust_atlas, math::Vector2D(-8, -8));
		
	} else if (animtions_name == "animations/dash_dust.json") {
		return std::make_tuple(dash_dust_atlas, math::Vector2D(-16, -32));

	} else if (animtions_name == "animations/buster.json") {
		return std::make_tuple(buster_atlas, math::Vector2D(-16, -32));

	} else if (animtions_name == "animations/buster-charge.json") {
		return std::make_tuple(buster_charge_atlas, math::Vector2D(-4.5, -4.5));
	}

	throw std::invalid_argument("unknown animations name");
}


template<class Callable>
void RenderAnimatedObject(ugdk::graphic::Canvas& canvas, const backend::AtlasObject& object,
						  Callable post_render) {
	auto data = get_data_for(object.animations_name());

	auto atlas = std::get<0>(data);
	auto&& frame = object.CurrentAnimationFrame();
	if (!frame.effect().visible())
		return;

	canvas.PushAndCompose(
		math::Geometry(
			object.position(),
			math::Vector2D(object.direction(), 1)
		));
	canvas.PushAndCompose(math::Geometry(std::get<1>(data)));
	graphic::immediate::Rectangle(canvas, math::Vector2D(), atlas, frame);
	canvas.PopGeometry();

	post_render();
	canvas.PopGeometry();
}

void RenderPlayer(ugdk::graphic::Canvas& canvas, const backend::PlayerCharacter& player) {
	auto x = player.charge_sprites();
	RenderAnimatedObject(canvas, player, [&] {
		for (const auto& child : player.charge_sprites())
			RenderAnimatedObject(canvas, child, [] {});
	});
}

}

ObjectViewer::ObjectViewer(backend::ServerProxy* server)
    : server_(server)
{
	populate_atlas();
}

void ObjectViewer::Render(ugdk::graphic::Canvas & canvas)
{
	RenderPlayer(canvas, server_->player_character());

	for (const auto& obj : server_->effects()) {
		RenderAnimatedObject(canvas, obj, [] {});
	}
	for (const auto& obj : server_->bullets()) {
		RenderAnimatedObject(canvas, obj, [] {});
	}
}

} // namespace frontend
