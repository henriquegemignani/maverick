
#include "backend/serverproxy.h"

#include "server/platformingcore.h"
#include "backend/ugdktiledfileloader.h"
#include <ugdk/filesystem/module.h>
#include "playercharacter.h"
#include <ugdk/input/module.h>

namespace backend {

using namespace ugdk;

struct ServerProxyImpl {
	explicit ServerProxyImpl(ServerProxy*);

	std::unique_ptr<server::PlatformingCore> core_;
    PlayerCharacter player_character_;
    bool frame_stepping_;
    std::list<Effect> effects_;

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
    for (auto& effect : effects_)
        effect.Update();

    effects_.remove_if([](const Effect& effect) { return effect.finished(); });
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

void ServerProxy::AddDustAt(const ugdk::math::Vector2D& position) {
    impl_->effects_.emplace_back(position, "animations/dust.json");
}

ServerProxy::ServerProxy()
    : impl_(new ServerProxyImpl(this))
{}

ServerProxy::~ServerProxy() {}

} // namespace frontend