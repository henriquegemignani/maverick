
#include "frontend/serverproxy.h"

#include "server/platformingcore.h"

namespace frontend {

struct ServerProxyImpl {
	ServerProxyImpl();

	std::unique_ptr<server::PlatformingCore> core_;
};

std::unique_ptr<tiled::Map> load_map() {
	return tiled::Map::ReadFromFile("mmx1/introstage.json");
}

ServerProxyImpl::ServerProxyImpl() 
	: core_(new server::PlatformingCore(load_map())) {}

namespace {
	std::unique_ptr<ServerProxy> reference_;
}

ServerProxy* ServerProxy::reference() {
	if (!reference_)
		reference_.reset(new ServerProxy);
	return reference_.get();
}

const tiled::Map* ServerProxy::map() const {
	return impl_->core_->map();
}

ServerProxy::ServerProxy() : impl_(new ServerProxyImpl) {}
ServerProxy::~ServerProxy() {}

} // namespace frontend