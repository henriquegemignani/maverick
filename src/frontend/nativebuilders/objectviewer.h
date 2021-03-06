#ifndef MAVERICK_FRONTEND_NATIVEBUILDERS_EFFECTVIEWER_H_
#define MAVERICK_FRONTEND_NATIVEBUILDERS_EFFECTVIEWER_H_

#include "backend/serverproxy.h"
#include "backend/atlasobject.h"
#include <ugdk/graphic/sprite.h>
#include <ugdk/graphic/primitive.h>

namespace frontend {

class ObjectViewer
{
public:
    explicit ObjectViewer(backend::ServerProxy*);

    void Render(ugdk::graphic::Canvas&);

private:
    backend::ServerProxy* server_;
};

} // namespace frontend

#endif // MAVERICK_FRONTEND_NATIVEBUILDERS_EFFECTVIEWER_H_
