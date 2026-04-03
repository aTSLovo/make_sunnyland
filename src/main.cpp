#include "engine/core/GameApp.h"
#include <spdlog/spdlog.h>

int main(int, char**) {
    spdlog::set_level(spdlog::level::debug);

    engine::core::GameApp game;
    game.run();
    return 0;
}