
#include "frontend/nativebuilders.h"

#include "frontend/serverproxy.h"
#include <ugdk/action/scene.h>

namespace frontend {

namespace nativebuilders {

std::unique_ptr<ugdk::action::Scene> GameViewerScene() {
	ServerProxy* server_proxy = ServerProxy::reference();
    return std::make_unique<ugdk::action::Scene>();
}

} // namespace nativebuilders
} // namespace frontend
