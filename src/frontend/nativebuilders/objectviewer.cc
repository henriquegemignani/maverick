
#include "frontend/nativebuilders/objectviewer.h"

#include "backend/playercharacter.h"
#include "backend/gameconstants.h"

#include <ugdk/action/scene.h>
#include <ugdk/graphic/canvas.h>
#include <ugdk/graphic/textureatlas.h>
#include <ugdk/graphic/primitivesetup.h>
#include <ugdk/resource/module.h>
#include <ugdk/graphic/immediate.h>

#include "backend/ugdktiledfileloader.h"
#include "libjson.h"
#include "tiled-reader/exceptions.h"


namespace frontend {
  
using namespace ugdk;

namespace {

class ChargeSprites : public backend::AtlasObject {
public:
    ChargeSprites(const std::string& animations_name, const ugdk::math::Vector2D& position, const std::string& frame_name)
        : AtlasObject(animations_name, position)
        , frame_name_(frame_name) {
    }

    action::SpriteAnimationFrame CurrentAnimationFrame() const override {
        auto ret = action::SpriteAnimationFrame(frame_name_);
        if (frame_name_ == "x1_lv1_2.png")
            ret.geometry().set_offset(math::Vector2D(-0.5, -0.5));
        return ret;
    }

private:
    std::string frame_name_;
};

graphic::TextureAtlas* x_atlas = nullptr;
graphic::TextureAtlas* dust_atlas = nullptr;
graphic::TextureAtlas* dash_dust_atlas = nullptr;
graphic::TextureAtlas* buster_atlas = nullptr;
graphic::TextureAtlas* buster_charge_atlas = nullptr;

std::array<std::vector<std::vector<ChargeSprites>>, 5> charge_sprites;


void initialize_charge_sprites() {
    auto loader = backend::UgdkTiledFileLoader();

    auto contents = json_string(loader.GetContents(loader.OpenFile("x1-charge.json")).c_str());
    if (!libjson::is_valid(contents))
        throw tiled::BaseException("Invalid json: x1-charge.json\n");

    auto json_root = libjson::parse(contents);

    charge_sprites[0].clear();
    charge_sprites[0].emplace_back();

    charge_sprites[1].clear();

    auto lv1 = json_root["lv1"];
    for (auto animation_frame : lv1) {
        charge_sprites[1].emplace_back();
        for (auto effect : animation_frame["effects"]) {
            math::Vector2D position(effect["position"].at(0).as_float(), effect["position"].at(1).as_float());
            charge_sprites[1].back().emplace_back("animations/buster-charge.json", position, effect["name"].as_string());
        }
    }
}


void populate_atlas() {
	if (x_atlas) return;

	x_atlas = resource::GetTextureAtlasFromFile("spritesheets/x");
	dust_atlas = resource::GetTextureAtlasFromFile("spritesheets/dust");
	dash_dust_atlas = resource::GetTextureAtlasFromFile("spritesheets/dash_dust");
	buster_atlas = resource::GetTextureAtlasFromFile("spritesheets/buster");
	buster_charge_atlas = resource::GetTextureAtlasFromFile("spritesheets/buster-charge");
    initialize_charge_sprites();
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
		return std::make_tuple(buster_charge_atlas, math::Vector2D(-4.5, -20.5));
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
    post_render(canvas);
	canvas.PopGeometry();
}

void RenderPlayer(ugdk::graphic::Canvas& canvas, const backend::PlayerCharacter& player) {
    auto shoot_charge_ticks = player.shoot_charge_ticks();
    auto charge_level = backend::PlayerCharacter::ChargeLevel(shoot_charge_ticks);
    auto ticks_in_level = shoot_charge_ticks - backend::constants::kBusterLevelChargeCount[charge_level];
    const auto& charge_level_animation = charge_sprites[charge_level];
    const auto& charge_sprites = charge_level_animation[ticks_in_level % charge_level_animation.size()];

	RenderAnimatedObject(canvas, player, [charge_sprites](graphic::Canvas& canvas) {
		for (const auto& child : charge_sprites)
			RenderAnimatedObject(canvas, child, [](graphic::Canvas& canvas) {});
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
		RenderAnimatedObject(canvas, obj, [](graphic::Canvas& canvas) {});
	}
	for (const auto& obj : server_->bullets()) {
		RenderAnimatedObject(canvas, obj, [](graphic::Canvas& canvas) {});
	}
}

} // namespace frontend
