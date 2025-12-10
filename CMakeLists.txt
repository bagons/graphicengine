cmake_minimum_required(VERSION 3.2)  # CMake version check
project(gameengine)

set(CMAKE_CXX_STANDARD 20)

# Including GLFW
#set(SOURCE_FILES gameengine.cpp)
add_subdirectory(ext/glfw)
include_directories(glfw/include)

# Including GLAD
add_subdirectory(ext/glad)
include_directories(ext/glad/include)

# Including GLM
add_subdirectory(ext/glm)
include_directories(ext/glm/glm)

include_directories(include)

add_library(${PROJECT_NAME} STATIC
        src/gameengine.cpp ext/glad/glad.c
        src/utils.cpp
        include/utils.h
        src/things.cpp
        include/things.hpp
        src/input.cpp
        include/input.hpp
        src/meshes.cpp
        include/meshes.hpp
        src/shaders.cpp
        include/shaders.hpp
        src/textures.cpp
        include/textures.hpp
        src/coordinates.cpp
        include/coordinates.h
        src/renderer.cpp
        include/renderer.hpp
        include/gereferences.hpp
)

target_link_libraries(${PROJECT_NAME} PUBLIC glfw glm)