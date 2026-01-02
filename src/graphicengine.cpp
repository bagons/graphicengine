#include <iostream>
#include "graphicengine.hpp"


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // we work with the premiss that we have only one window
    glViewport(0, 0, width, height);
    std::cout << width << "x" << height << std::endl;
    const auto engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    engine->window.width = width;
    engine->window.height = height;

    // update all render layers
    for (render_layer_container::const_iterator it = engine->render_layers.begin(); it != engine->render_layers.end(); ++it) {
        it->second->change_resolution(width, height);
    }
}

    /* Create a windowed mode window and its OpenGL context */
Window::Window(const char* title, const int _width, const int _height) {
    if (glfwGetCurrentContext()) {
        std::cerr << "A WINDOW ALREADY EXISTS, ONLY ONE WINDOW ALLOWED! -your engine <3" << std::endl;
        return;
    }

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return;
    }

    width = _width;
    height = _height;
    glfwwindow = glfwCreateWindow(width, height, title, nullptr, nullptr);
    glfwSetFramebufferSizeCallback(glfwwindow, framebuffer_size_callback);
}

Window::~Window() {
    if (!glfwGetCurrentContext()) {
        std::cerr << "GLFW already terminated!" << std::endl;
        return;
    }
    glfwTerminate();
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


void Engine::update() {
    things_container::const_iterator it;

    for (it = things.begin(); it != things.end(); it++) {
        it->second->update();
    }

    // input update to correctly adjust just pressed keys
    input.update();
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

void Engine::set_bindless_texture_support(const bool support_bindless_textures) {
    bindless_texture_supported = support_bindless_textures;
    shaders.bindless_textures_supported = support_bindless_textures;
}

bool Engine::are_bindless_textures_supported() const {
    return bindless_texture_supported;
}

// run to start engine
Engine::Engine(const char* display_name, const int screen_width, const int screen_height) : window(display_name, screen_width, screen_height) {
    std::cout << window.width << "x" << window.height << std::endl;
    // init window library


    // handles window initialization
    // error when creating a window
    if (!window.glfwwindow) {
        std::cerr << "Failed to create window" << std::endl;
        return;
    }

    // connect window to engine class
    glfwSetWindowUserPointer(window.glfwwindow, &ge);
    // select windows context for rendering (possibile to select another window if rendering onto more windows, not supported yet, but why tho)
    window.select();

    // setup engine input handling
    input.init();
    input.connect_callbacks(window.glfwwindow);

    // LOAD OPEN GL
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }

    if (GLAD_GL_ARB_bindless_texture) {
        ge.set_bindless_texture_support(true);
        std::cout << "Bindless texture supported!" << std::endl;
    } else {
        ge.set_bindless_texture_support(false);
        std::cout << "Bindless texture NOT supported!" << std::endl;
    }

    // engine setup
    init_render_pipeline();

    // base material setup
    shaders.setup_base_materials();

    std::cout << "finishing game engine setup" << std::endl;
}

Engine::~Engine() {
    glDeleteBuffers(1, &ge.camera_matrix_ubo);
}


