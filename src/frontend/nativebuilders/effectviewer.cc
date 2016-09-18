
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

void populate_atlas() {
	if (dust_atlas) return;

	dust_atlas = resource::GetTextureAtlasFromFile("spritesheets/dust");
	dash_dust_atlas = resource::GetTextureAtlasFromFile("spritesheets/dash_dust");
}

graphic::TextureAtlas* get_atlas(backend::Effect::Type type) {
	switch(type) {
	case backend::Effect::Type::DUST:
		return dust_atlas;
		break;
	case backend::Effect::Type::DASH_DUST:
		return dash_dust_atlas;
		break;
	default:
		throw std::exception("unsupported effect type");
		break;
	}
}

math::Vector2D hotspot_for(backend::Effect::Type type) {
	switch (type) {
	case backend::Effect::Type::DUST:
		return math::Vector2D(-8.0, -8.0);
		break;
	case backend::Effect::Type::DASH_DUST:
		return math::Vector2D(-16.0, -32.0);
		break;
	default:
		throw std::exception("unsupported effect type");
		break;
	}
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
		auto atlas = get_atlas(effect.type());
		auto&& frame = effect.player().current_animation_frame();
		auto&& piece = atlas->PieceAt(frame.atlas_frame_name());
		//primitive_.set_visualeffect(frame.effect());
		
		graphic::VertexDataManipulation::SetUsingSpriteFrameInformation(*primitive_.vertexdata(), math::Vector2D(), frame, piece);

		// Send the texture to the GPU
		auto sprite_unit = ugdk::graphic::manager()->ReserveTextureUnit(atlas->texture());
		canvas.SendUniform("drawable_texture", sprite_unit);

		canvas.PushAndCompose(
			math::Geometry(
				effect.position(),
				math::Vector2D(effect.direction(), 1)
			));
		canvas.PushAndCompose(math::Geometry(hotspot_for(effect.type())));
		graphic::PrimitiveSetup::Sprite::Render(primitive_, canvas);
        canvas.PopGeometry();
		canvas.PopGeometry();
    }
}

} // namespace frontend