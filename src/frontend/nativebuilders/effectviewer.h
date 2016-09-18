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

    void Render(ugdk::graphic::Canvas&);

private:
	void RenderAnimatedObject(ugdk::graphic::Canvas&, const backend::AnimatedObject&);

    backend::ServerProxy* server_;
    ugdk::graphic::Primitive primitive_;
};

} // namespace frontend

#endif // MAVERICK_FRONTEND_NATIVEBUILDERS_EFFECTVIEWER_H_
