#ifndef MAVERICK_FRONTEND_NATIVEBUILDERS_EFFECTVIEWER_H_
#define MAVERICK_FRONTEND_NATIVEBUILDERS_EFFECTVIEWER_H_

#include "backend/serverproxy.h"
#include <ugdk/graphic/sprite.h>
#include <ugdk/graphic/primitive.h>

namespace frontend {

class EffectViewer
{
public:
    explicit EffectViewer(backend::ServerProxy*);

    void Render(ugdk::graphic::Canvas&) const;

private:
    backend::ServerProxy* server_;
    ugdk::graphic::Primitive primitive_;
    ugdk::graphic::PrimitiveControllerSprite* controller_;
};

} // namespace frontend

#endif // MAVERICK_FRONTEND_NATIVEBUILDERS_EFFECTVIEWER_H_
