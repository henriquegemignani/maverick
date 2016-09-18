
#include "frontend/nativebuilders/effectviewer.h"

#include <ugdk/action/scene.h>
#include <ugdk/graphic/canvas.h>
#include <ugdk/graphic/module.h>
#include <ugdk/graphic/primitive.h>
#include <ugdk/graphic/textureatlas.h>
#include <ugdk/graphic/primitivesetup.h>
#include <ugdk/graphic/sprite.h>
#include <ugdk/resource/module.h>
#include <ugdk/graphic/primitivesetup.h>
#include <ugdk/ui/drawable/texturedrectangle.h>
#include <ugdk/input/events.h>
#include <ugdk/input/module.h>
#include <ugdk/input/joystick.h>
#include <algorithm>

namespace frontend {
  
using namespace ugdk;

namespace {
graphic::TextureAtlas* dust_atlas = nullptr;
graphic::TextureAtlas* dash_dust_atlas = nullptr;
graphic::TextureAtlas* buster_atlas = nullptr;

void populate_atlas() {
	if (dust_atlas) return;

	dust_atlas = resource::GetTextureAtlasFromFile("spritesheets/dust");
	dash_dust_atlas = resource::GetTextureAtlasFromFile("spritesheets/dash_dust");
	buster_atlas = resource::GetTextureAtlasFromFile("spritesheets/buster");
}

std::tuple<graphic::TextureAtlas*, math::Vector2D>
get_data_for(const std::string& animtions_name) {

	if (animtions_name == "animations/dust.json") {
		return std::make_tuple(dust_atlas, math::Vector2D(-8, -8));
		
	} else if (animtions_name == "animations/dash_dust.json") {
		return std::make_tuple(dash_dust_atlas, math::Vector2D(-16, -32));

	} else if (animtions_name == "animations/buster.json") {
		return std::make_tuple(buster_atlas, math::Vector2D(-32, -32));
	}

	throw std::exception("unknown animations name");
}

}

EffectViewer::EffectViewer(backend::ServerProxy* server)
    : server_(server)
    , primitive_(nullptr, nullptr)
{
	populate_atlas();
	primitive_.set_vertexdata(graphic::CreateVertexDataWithSpecification(graphic::PrimitiveSetup::Sprite::vertexdata_specification));
}

void EffectViewer::Render(ugdk::graphic::Canvas & canvas)
{
    for (const auto& effect : server_->effects()) {
		RenderAnimatedObject(canvas, effect);
    }
}

void EffectViewer::RenderAnimatedObject(ugdk::graphic::Canvas& canvas, const backend::AnimatedObject& object) {
	auto data = get_data_for(object.animations_name());

	auto atlas = std::get<0>(data);
	auto&& frame = object.player().current_animation_frame();
	auto&& piece = atlas->PieceAt(frame.atlas_frame_name());
	//primitive_.set_visualeffect(frame.effect());

	graphic::VertexDataManipulation::SetUsingSpriteFrameInformation(*primitive_.vertexdata(), math::Vector2D(), frame, piece);

	// Send the texture to the GPU
	auto sprite_unit = ugdk::graphic::manager()->ReserveTextureUnit(atlas->texture());
	canvas.SendUniform("drawable_texture", sprite_unit);

	canvas.PushAndCompose(
		math::Geometry(
			object.position(),
			math::Vector2D(object.direction(), 1)
		));
	canvas.PushAndCompose(math::Geometry(std::get<1>(data)));
	graphic::PrimitiveSetup::Sprite::Render(primitive_, canvas);
	canvas.PopGeometry();
	canvas.PopGeometry();
}

} // namespace frontend