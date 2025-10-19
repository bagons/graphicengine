cmake_minimum_required(VERSION 3.31)
project(game)

set(CMAKE_CXX_STANDARD 20)

add_executable(game main.cpp)

add_subdirectory(../../buil)
include_directories(engine/include)
target_link_libraries(game engine)


add_custom_command(TARGET ${PROJECT_NAME}  POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/res"
        "${CMAKE_CURRENT_BINARY_DIR}/res"
)