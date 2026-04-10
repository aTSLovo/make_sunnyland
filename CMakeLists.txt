# 标题
cmake_minimum_required(VERSION 3.10.0)
project(SunnyLand VERSION 0.1.0 LANGUAGES C CXX)

# 设置C++标准 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 设置编译选项
if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 设置编译输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR})

set(TARGET ${PROJECT_NAME}-${CMAKE_SYSTEM_NAME})

# 查找并载入Cmake预设
find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)
find_package(SDL3_mixer REQUIRED)
find_package(SDL3_ttf REQUIRED)
find_package(glm REQUIRED)
find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)

# add_executable(${TARGET}
#                 src/main.cpp
#                 src/engine/core/gameApp.cpp
#                 src/engine/core/time.cpp
#                 src/engine/core/config.cpp
#                 src/engine/core/context.cpp
#                 src/engine/resource/fontManager.cpp
#                 src/engine/resource/musicManager.cpp
#                 src/engine/resource/textureManager.cpp
#                 src/engine/resource/resourceManager.cpp
#                 src/engine/render/camera.cpp
#                 src/engine/render/renderer.cpp
#                 src/engine/input/inputManager.cpp
#                 src/engine/object/gameObject.cpp
#                 src/engine/component/transformComponent.cpp
#                 src/engine/component/spriteComponent.cpp
#                 src/engine/scene/scene.cpp
#                 src/engine/scene/sceneManager.cpp
# )
add_executable(${TARGET})
set(sources src/main.cpp
            src/engine/core/gameApp.cpp
            src/engine/core/time.cpp
            src/engine/core/config.cpp
            src/engine/core/context.cpp
            src/engine/resource/fontManager.cpp
            src/engine/resource/musicManager.cpp
            src/engine/resource/textureManager.cpp
            src/engine/resource/resourceManager.cpp
            src/engine/render/camera.cpp
            src/engine/render/renderer.cpp
            src/engine/input/inputManager.cpp
            src/engine/object/gameObject.cpp
            src/engine/component/transformComponent.cpp
            src/engine/component/spriteComponent.cpp
            src/engine/component/parallaxComponent.cpp
            src/engine/component/tilelayerComponent.cpp
            src/engine/component/physicsComponent.cpp
            src/engine/scene/scene.cpp
            src/engine/scene/sceneManager.cpp
            src/engine/scene/levelLoader.cpp
            src/engine/physics/physicsEngine.cpp
            src/game/scene/gameScene.cpp
)

# 添加可执行文件
target_sources(${TARGET} PUBLIC ${sources})

# 链接库
target_link_libraries(${TARGET}
                        ${SDL3_LIBRARIES}
                        SDL3_image::SDL3_image
                        SDL3_mixer::SDL3_mixer
                        SDL3_ttf::SDL3_ttf
                        glm::glm
                        spdlog::spdlog
                        nlohmann_json::nlohmann_json
                        )