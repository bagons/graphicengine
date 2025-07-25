#include <iostream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "utils.h"
#include "gameengine.hpp"
#include "things.hpp"


const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;


Window::Window() {
    glfwwindow = nullptr;
}

    /* Create a windowed mode window and its OpenGL context */
Window::Window(const int width, const int height, const char* title) {
    glfwwindow = glfwCreateWindow(width, height, title, nullptr, nullptr);
}

void Window::select() const {
    glfwMakeContextCurrent(glfwwindow);
}

void Engine::load_base_vertex_shader() {
    base_vertex_shader = compile_shader_from_file("engine/base_shaders/vertex.glsl", GL_VERTEX_SHADER);
}

void Engine::init_render_pipeline() {
    load_base_vertex_shader();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void Engine::render(const int camera_index) {
    glClearColor(0.4f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    for (Thing * thing : things) {
        if (thing->renderable) {
            thing->render(cameras[camera_index]);
        }
    }
}

void Engine::update() const {
    for (Thing * thing : things) {
        if (thing->updatable) {
            thing->update();
        }
    }
}

void Engine::send_it_to_window() {
    /* Swap front and back buffers */
    glfwSwapBuffers(window.glfwwindow);
    /* Poll for and process events */
    glfwPollEvents();
}

bool Engine::is_running() const {
    return !glfwWindowShouldClose(window.glfwwindow);
}

Engine::~Engine() {
    glDeleteShader(base_vertex_shader);
}

Engine ge;

// run to start engine
Engine* gameengine(const char* game_name) {
    /* Initialize the library */
    if (!glfwInit())
        return nullptr;

    Window window(SCREEN_WIDTH, SCREEN_HEIGHT, game_name);
    // error when creating a window
    if (!window.glfwwindow)
        return nullptr;

    ge.window = window;
    window.select();

    // load glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return nullptr;
    }

    // engine setup
    ge.init_render_pipeline();
    std::cout << "finishing game engine setup" << std::endl;
    return &ge;
}

// run after stopping engine
int noengine() {
    glfwTerminate();
    return 0;
}

