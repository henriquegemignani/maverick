#ifndef MAVERICK_FRONTEND_FRONTEND_H_
#define MAVERICK_FRONTEND_FRONTEND_H_

#include <ugdk/system/configuration.h>

namespace frontend {

void PopuplateUGDKConfiguration(ugdk::system::Configuration&);

/// Initializes necessary global state.
void Initialize();

/// Pushes the initial scene to the stack.
void Start();

} // namespace frontend

#endif // MAVERICK_FRONTEND_FRONTEND_H_
