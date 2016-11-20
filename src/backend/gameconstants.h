#ifndef MAVERICK_BACKEND_GAMECONSTANTS_H_
#define MAVERICK_BACKEND_GAMECONSTANTS_H_

#include <array>

namespace backend {
namespace constants {

constexpr double kGravity = 0.25;
constexpr double kTerminalSpeed = 5.75;
constexpr double kJumpSpeed = 5.0;
constexpr double kWalkingSpeed = 1.5;
constexpr double kWallSlidingSpeed = 2.0;
constexpr double kWallKickJumpSpeed = 4.0;
constexpr double kDashingSpeed = 3.5;
constexpr int kDashLength = 33;
constexpr int kShootAnimationLength = 16;
constexpr int kBusterLevel1ChargeCount = 20;
constexpr int kBusterLevel2ChargeCount = 80;
constexpr int kBusterLevel3ChargeCount = 140;
constexpr int kBusterLevel4ChargeCount = 200;

constexpr std::array<int, 5> kBusterLevelChargeCount = {
    0,
    kBusterLevel1ChargeCount,
    kBusterLevel2ChargeCount,
    kBusterLevel3ChargeCount,
    kBusterLevel4ChargeCount
};

}
} // namespace backend

#endif // MAVERICK_BACKEND_GAMECONSTANTS_H_
