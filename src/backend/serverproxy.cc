
#include "backend/serverproxy.h"

#include "server/platformingcore.h"
#include "backend/ugdktiledfileloader.h"
#include <ugdk/filesystem/module.h>
#include "playercharacter.h"
#include <ugdk/input/module.h>
#include "backend/collision.h"

namespace backend {

using namespace ugdk;

struct ServerProxyImpl {
	explicit ServerProxyImpl(ServerProxy*);

	Collision collision_;
	std::unique_ptr<server::PlatformingCore> core_;
    PlayerCharacter player_character_;
    bool frame_stepping_;
    std::list<Effect> effects_;
	std::list<Bullet> bullets_;
	void Tick();
};

std::unique_ptr<tiled::Map> load_map() {
	return tiled::Map::ReadFromFile("mmx1/introstage/map.json", backend::UgdkTiledFileLoader());
}

ServerProxyImpl::ServerProxyImpl(ServerProxy* proxy)
	: core_(new server::PlatformingCore(load_map()))
    , player_character_(proxy)
    , frame_stepping_(false)
{}

void ServerProxyImpl::Tick() {
    if (input::manager()->keyboard().IsPressed(input::Scancode::P))
        frame_stepping_ = !frame_stepping_;

    if (frame_stepping_ && !input::manager()->keyboard().IsPressed(input::Scancode::F))
        return;

    player_character_.Update();
    
	for (auto& obj : effects_)
		obj.Update();
	effects_.remove_if([](const Effect& obj) { return obj.finished(); });

	for (auto& obj : bullets_)
		obj.Update();
	bullets_.remove_if([](const Bullet& obj) { return obj.finished(); });
}

// =========================================

namespace {
	std::unique_ptr<ServerProxy> reference_;
}

ServerProxy* ServerProxy::reference() {
	if (!reference_)
		reference_.reset(new ServerProxy);
	return reference_.get();
}

void ServerProxy::Tick() {
    impl_->Tick();
}

const tiled::Map* ServerProxy::map() const {
	return impl_->core_->map();
}

PlayerCharacter& ServerProxy::player_character() {
    return impl_->player_character_;
}

const std::list<Effect>& ServerProxy::effects() const {
    return impl_->effects_;
}

const std::list<Bullet>& ServerProxy::bullets() const {
	return impl_->bullets_;
}

Collision& ServerProxy::collision() {
	return impl_->collision_;
}

void ServerProxy::AddEffectAt(const ugdk::math::Vector2D& position, Effect::Type type) {
	impl_->effects_.emplace_back(position, type);
}

void ServerProxy::AddEffectAt(const ugdk::math::Vector2D& position, Effect::Type type, int direction) {
	AddEffectAt(position, type);
	impl_->effects_.back().set_direction(direction);
}

void ServerProxy::ShootBulletAt(const ugdk::math::Vector2D& position, Bullet::Type type, int direction) {
	impl_->bullets_.emplace_back(position, type);
	impl_->bullets_.back().set_direction(direction);
}

ServerProxy::ServerProxy()
    : impl_(new ServerProxyImpl(this))
{
	impl_->player_character_.SetupCollision();
	
	auto _map = map();
	auto& L = impl_->collision_.lua();
	auto& layer = _map->layers()[0];

	for (auto col = 0; col < layer.width(); ++col) {
		for (auto row = 0; row < layer.height(); ++row) {
			auto tile = layer.tile_at(col, row);
			
			auto& properties = _map->tileproperties_for(tile);
			auto f = properties.find("solid");
			if (f != properties.end() && f->second.bool_value()) {
				L["world"]["add"](L["world"],
								  L.create_table(),
								  col * _map->tile_width(),
								  row * _map->tile_height(),
								  _map->tile_width(),
								  _map->tile_height());
			}
		}
	}
}

ServerProxy::~ServerProxy() {}

} // namespace frontend