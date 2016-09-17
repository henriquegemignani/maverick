
#include <ugdk/system/config.h>
#include <ugdk/system/engine.h>
#include <ugdk/resource/module.h>
#include <ugdk/debug/log.h>

#include <string>
#include <cassert>
#include "SDL.h"
#ifdef main
#undef main
#endif

#include "frontend/frontend.h"

void ExitWithFatalError(const std::string& msg) {
	if (SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", msg.c_str(), nullptr) < 0) {
		ugdk::debug::Log(ugdk::debug::LogLevel::ERROR, "Maverick", msg);
	}
	assert(false);
	exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
	ugdk::system::Configuration engine_config;
	frontend::PopuplateUGDKConfiguration(engine_config);
    engine_config.base_path = MAVERICK_DATA_PATH;

	if (!ugdk::system::Initialize(engine_config))
		ExitWithFatalError("Could not initialize UGDK.");

	frontend::Initialize();
	frontend::Start();

	ugdk::system::Run();
	ugdk::system::Release();
    return 0;
}