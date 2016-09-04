
#include "frontend/frontend.h"

#include "frontend/nativebuilders.h"
#include "frontend/debugtools.h"

#include <ugdk/action/scene.h>
#include <ugdk/system/engine.h>
#include <ugdk/text/module.h>
#include <ugdk/input/events.h>

namespace frontend {

void PopuplateUGDKConfiguration(ugdk::system::Configuration& config) {
    //config.canvas_size = settings->resolution_vector();
    config.windows_list[0].title = "Maverick";
	config.windows_list[0].vsync = true;
    //config.windows_list[0].size = settings->resolution_vector();
    //config.windows_list[0].fullscreen = settings->fullscreen();
}

void Initialize() {
    //ugdk::text::manager()->RegisterLanguage("en_US", "text/lang_en.txt");
    //ugdk::text::manager()->RegisterLanguage("pt_BR", "text/lang_pt_br.txt");
    //ugdk::text::manager()->Setup(Settings::reference()->language_name());
    ugdk::system::GlobalEventHandler().AddObjectListener(DebugToolsListener());
}

void Start() {
    ugdk::system::PushSceneFactory(nativebuilders::GameViewerScene);
}

} // namespace frontend