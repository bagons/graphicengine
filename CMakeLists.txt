cmake_minimum_required(VERSION 3.31)  # CMake version check
project(graphicengine)

set(CMAKE_CXX_STANDARD 20)

# Including GLFW
add_subdirectory(ext/glfw)

# Including GLAD
add_subdirectory(ext/glad)

# Including GLM
add_subdirectory(ext/glm)

add_library(${PROJECT_NAME} STATIC
        src/graphicengine.cpp ext/glad/glad.c
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

target_include_directories(graphicengine PUBLIC
        include
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/glfw/include
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/glad/include
        ${CMAKE_CURRENT_SOURCE_DIR}/ext/glm/glm
)

target_link_libraries(${PROJECT_NAME} PUBLIC glfw glm)