
#include "frontend/nativebuilders.h"

#include <ugdk/action/scene.h>

namespace frontend {
namespace nativebuilders {



std::unique_ptr<ugdk::action::Scene> GameScene() {
    return std::make_unique<ugdk::action::Scene>();
}

} // namespace nativebuilders
} // namespace frontend
