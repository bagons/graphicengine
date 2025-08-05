#include <iostream>

#include "gameengine.hpp"
#include "utils.h"


const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // we work with the premiss that we have only one window
    glViewport(0, 0, width, height);
    std::cout << width << "x" << height << std::endl;
    ge.window.width = width;
    ge.window.height = height;
    // WARNING taking into account we have only one camera
    ge.cameras[0]->change_resolution(width, height);
}

Window::Window() {
    glfwwindow = nullptr;
}

    /* Create a windowed mode window and its OpenGL context */
Window::Window(const int _width, const int _height, const char* title) {
    width = _width;
    height = _height;
    glfwwindow = glfwCreateWindow(width, height, title, nullptr, nullptr);
    glfwSetFramebufferSizeCallback(glfwwindow, framebuffer_size_callback);
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
    glEnable(GL_DEPTH_TEST);
}

void Engine::render(const int camera_index) {
    glClearColor(0.4f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
    // input update to correctly adjust just pressed keys
    ge.input.update();
}

void Engine::send_it_to_window() {
    /* Swap front and back buffers */
    glfwSwapBuffers(window.glfwwindow);
    /* Poll for and process events */
    glfwPollEvents();

    frame_delta = static_cast<float>(glfwGetTime() - last_game_time);
    last_game_time = glfwGetTime();
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
    // init window library
    if (!glfwInit())
        return nullptr;

    // handles window initialization
    Window window(SCREEN_WIDTH, SCREEN_HEIGHT, game_name);
    // error when creating a window
    if (!window.glfwwindow)
        return nullptr;

    // connect window to engine class
    ge.window = window;

    // select windows context for rendering (possibile to select another window if rendering onto more windows, not supported yet, but why tho)
    window.select();

    // setup engine input handling
    ge.input.connect_callbacks(window.glfwwindow);

    // load glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return nullptr;
    }

    if (GLAD_GL_ARB_bindless_texture) {
        std::cout << "Bindless texture supported!" << std::endl;
    } else {
        std::cerr << "Bindless texture NOT supported!" << std::endl;
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

