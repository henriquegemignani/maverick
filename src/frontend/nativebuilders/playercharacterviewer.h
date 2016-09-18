#ifndef MAVERICK_FRONTEND_NATIVEBUILDERS_PLAYERCHARACTERVIEWER_H_
#define MAVERICK_FRONTEND_NATIVEBUILDERS_PLAYERCHARACTERVIEWER_H_

#include "backend/playercharacter.h"

#include <memory>
#include <ugdk/graphic/sprite.h>
#include <ugdk/graphic/primitive.h>
#include <ugdk/input/events.h>
#include <ugdk/system/eventhandler.h>
#include <ugdk/action/observer.h>

namespace frontend {

class PlayerCharacterViewer
{
public:
    explicit PlayerCharacterViewer(backend::PlayerCharacter* player_character);

    void Render(ugdk::graphic::Canvas&) const;

private:
    backend::PlayerCharacter* player_character_;
    ugdk::graphic::Primitive primitive_;
    ugdk::graphic::PrimitiveControllerSprite* controller_;
};

} // namespace frontend

#endif // MAVERICK_FRONTEND_NATIVEBUILDERS_PLAYERCHARACTERVIEWER_H_
