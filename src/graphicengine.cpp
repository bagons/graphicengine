#include <iostream>
#include "graphicengine.hpp"


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // we work with the premiss that we have only one window
    glViewport(0, 0, width, height);
    std::cout << "ENGINE MESSAGE: Window size changed: " << width << "x" << height << std::endl;
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
        std::cerr << "ENGINE ERROR: A window already exists, only one window allowed!" << std::endl;
        return;
    }

    if (!glfwInit()) {
        std::cerr << "ENGINE ERROR: Failed to initialize GLFW" << std::endl;
        return;
    }

    width = _width;
    height = _height;
    glfwwindow = glfwCreateWindow(width, height, title, nullptr, nullptr);
    glfwSetFramebufferSizeCallback(glfwwindow, framebuffer_size_callback);
}

Window::~Window() {
    if (!glfwGetCurrentContext()) {
        std::cerr << "ENGINE WARNING: GLFW already terminated! This condition SHOULD NOT have happened" << std::endl;
        return;
    }
    glfwTerminate();
}

void Window::select() const {
    glfwMakeContextCurrent(glfwwindow);
}

void Window::set_vsync(const bool _vsync) const {
    glfwMakeContextCurrent(glfwwindow);
    glfwSwapInterval(_vsync ? 1 : 0);
}

Thing *Engine::get_thing(const int id) {
    return things[id].get();
}

RenderPass *Engine::get_render_layer(const int id) {
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
        if (it->second->paused) {
            it->second->update();
        }
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
Engine::Engine(const char* display_name, const int screen_width, const int screen_height
    , unsigned int MAX_NR_POINT_LIGHTS, unsigned int MAX_NR_DIRECTIONAL_LIGHTS,  Lights::LightOverflowAction light_overflow_action) :
    window(display_name, screen_width, screen_height), lights(MAX_NR_POINT_LIGHTS, MAX_NR_DIRECTIONAL_LIGHTS) {

    // handles window initialization
    // error when creating a window
    if (!window.glfwwindow) {
        std::cerr << "ENGINE ERROR: Failed to create window" << std::endl;
        return;
    }

    // connect window to engine class
    glfwSetWindowUserPointer(window.glfwwindow, &ge);
    // select windows context for rendering (possibile to select another window if rendering onto more windows, not supported yet, but why tho)
    window.select();
    window.set_vsync(true);

    // setup engine input handling
    input.init();
    input.connect_callbacks(window.glfwwindow);

    // LOAD OPEN GL
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "ENIGNE ERROR: Failed to initialize GLAD" << std::endl;
        return;
    }

    if (GLAD_GL_ARB_bindless_texture) {
        ge.set_bindless_texture_support(true);
        std::cout << "ENGINE MESSAGE: Bindless texture supported!" << std::endl;
    } else {
        ge.set_bindless_texture_support(false);
        std::cout << "ENGINE MESSAGE: Bindless texture NOT supported!" << std::endl;
    }

    // engine setup
    init_render_pipeline();

    // base material setup
    shaders.setup_base_materials();

    // base meshes setup
    meshes.load_base_meshes();

    // init light system
    lights.init_central_light_system();

    std::cout << "ENGINE MESSAGE: Engine setup done" << std::endl;
}

void Engine::debug_message(const std::string &message) {
    std::cout << "ENGINE MESSAGE: " << message << std::endl;
}

void Engine::debug_error(const std::string &message) {
    std::cerr << "ENGINE ERROR: " << message << std::endl;
}

void Engine::debug_warning(const std::string &message) {
    std::cerr << "ENGINE WARNING: " << message << std::endl;
}

Engine::~Engine() {
    glDeleteBuffers(1, &camera_matrix_ubo);
}


