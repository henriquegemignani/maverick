
#include "frontend/debugtools.h"

#include "frontend/scenes/console.h"

#include <ugdk/input/events.h>
#include <ugdk/input/module.h>
#include <ugdk/system/compatibility.h>

namespace frontend {

namespace {

using namespace ugdk;

class DebugTools :
    public ugdk::system::Listener < ugdk::input::KeyPressedEvent >
{
    public:
    void Handle(const ugdk::input::KeyPressedEvent& ev) override {
        if (ev.scancode == input::Scancode::GRAVE && !frontend::scenes::Console::HasConsoleActive()) {
            ugdk::system::PushSceneFactory(&ugdk::MakeUnique<frontend::scenes::Console>);
        }
	    if (ev.scancode == input::Scancode::ESCAPE) {
			ugdk::system::CurrentScene().Finish();
	    }
    }
};

    std::unique_ptr<DebugTools> debug_tools_listener_;
}

ugdk::system::IListener* DebugToolsListener() {
    if (!debug_tools_listener_)
        debug_tools_listener_.reset(new DebugTools);
    return debug_tools_listener_.get();
}

} // namespace frontend