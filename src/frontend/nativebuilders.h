#ifndef MAVERICK_FRONTEND_NATIVEBUILDERS_H_
#define MAVERICK_FRONTEND_NATIVEBUILDERS_H_

#include <ugdk/action.h>
#include <memory>

namespace frontend {
namespace nativebuilders {

std::unique_ptr<ugdk::action::Scene> GameScene();

} // namespace nativebuilders
} // namespace frontend

#endif // MAVERICK_FRONTEND_NATIVEBUILDERS_H_
