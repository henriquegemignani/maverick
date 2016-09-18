
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

namespace frontend {
  
using namespace ugdk;

namespace {

graphic::TextureAtlas* x_atlas = nullptr;
graphic::TextureAtlas* dust_atlas = nullptr;
graphic::TextureAtlas* dash_dust_atlas = nullptr;
graphic::TextureAtlas* buster_atlas = nullptr;

void populate_atlas() {
	if (x_atlas) return;

	x_atlas = resource::GetTextureAtlasFromFile("spritesheets/x");
	dust_atlas = resource::GetTextureAtlasFromFile("spritesheets/dust");
	dash_dust_atlas = resource::GetTextureAtlasFromFile("spritesheets/dash_dust");
	buster_atlas = resource::GetTextureAtlasFromFile("spritesheets/buster");
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
	}

	throw std::exception("unknown animations name");
}

}

ObjectViewer::ObjectViewer(backend::ServerProxy* server)
    : server_(server)
    , primitive_(nullptr, nullptr)
{
	populate_atlas();
	primitive_.set_vertexdata(graphic::CreateVertexDataWithSpecification(graphic::PrimitiveSetup::Sprite::vertexdata_specification));
}

void ObjectViewer::Render(ugdk::graphic::Canvas & canvas)
{
	RenderAnimatedObject(canvas, server_->player_character());
	for (const auto& obj : server_->effects()) {
		RenderAnimatedObject(canvas, obj);
	}
	for (const auto& obj : server_->bullets()) {
		RenderAnimatedObject(canvas, obj);
	}
}

void ObjectViewer::RenderAnimatedObject(ugdk::graphic::Canvas& canvas, const backend::AnimatedObject& object) {
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