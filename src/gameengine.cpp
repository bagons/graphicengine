#include <iostream>

#include "gameengine.hpp"


const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // we work with the premiss that we have only one window
    glViewport(0, 0, width, height);
    std::cout << width << "x" << height << std::endl;
    ge.window.width = width;
    ge.window.height = height;

    // update all render layers
    for (render_layer_container::const_iterator it = ge.render_layers.begin(); it != ge.render_layers.end(); ++it) {
        it->second->change_resolution(width, height);
    }
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



Thing *Engine::get_thing(const int id) {
    return things[id].get();
}

RenderLayer *Engine::get_render_layer(const int id) {
    return render_layers[id].get();
}


void Engine::init_render_pipeline() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_DEPTH_TEST);

    // create a Uniform Buffer for camera data
    glGenBuffers(1, &camera_matrix_ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, camera_matrix_ubo);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, camera_matrix_ubo, 0, 2 * sizeof(glm::mat4));
}


void Engine::update() const {
    things_container::const_iterator it;

    for (it = things.begin(); it != things.end(); it++) {
        it->second->update();
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
    ge.input.init();
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

    // base material setup
    ge.shaders.setup_base_materials();

    std::cout << "finishing game engine setup" << std::endl;
    return &ge;
}

// run after stopping engine
int noengine() {
    glDeleteBuffers(1, &ge.camera_matrix_ubo);
    glfwTerminate();
    return 0;
}

