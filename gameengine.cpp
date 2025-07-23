#include <iostream>
#include <vector>
#include <glad/glad.h>
#include "utils.h"
#include <GLFW/glfw3.h>

#include "things.hpp"


const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;


class Window {
public:
    GLFWwindow* glfwwindow;
    int width{}, height{};

    Window() {
        glfwwindow = nullptr;
    }

    /* Create a windowed mode window and its OpenGL context */
    Window(const int width, const int height, const char* title) {
        glfwwindow = glfwCreateWindow(width, height, title, nullptr, nullptr);
    }


    /* Make the window's context current */
    void select() const {
        glfwMakeContextCurrent(glfwwindow);
    }
};


class Engine {
public:
    unsigned int base_vertex_shader = 0;
    std::vector<Camera *> cameras;
    std::vector<Thing *> things;
    Window window;

    void load_base_vertex_shader() {
        base_vertex_shader = compile_shader_from_file("engine/base_shaders/vertex.glsl", GL_VERTEX_SHADER);
    }

    void init_render_pipeline() {
        load_base_vertex_shader();

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    void render(const int camera_index) const{
        glClear(GL_COLOR_BUFFER_BIT);
        for (Thing * thing : things) {
            if (thing->renderable) {
                thing->render(cameras[camera_index]);
            }
        }
    }

    void send_it_to_window() const {
        /* Swap front and back buffers */
        glfwSwapBuffers(window.glfwwindow);

        /* Poll for and process events */
        glfwPollEvents();
    }

    bool is_running() const {
        return !glfwWindowShouldClose(window.glfwwindow);
    }

    ~Engine() {
        glDeleteShader(base_vertex_shader);
    }
};

Engine ge;

// run to start engine
Engine* gameengine(const char* game_name) {
    /* Initialize the library */
    if (!glfwInit())
        return nullptr;

    Window window = Window(SCREEN_WIDTH, SCREEN_HEIGHT, game_name);
    // error when creating a window
    if (!window.glfwwindow)
        return nullptr;

    ge.window = window;

    // load glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return nullptr;
    }

    // engine setup
    ge.init_render_pipeline();

     const std::vector<float> vertices = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         -0.5f,  0.5f, 0.0f,
         0.5f, 0.5f, 0.0f
    };

    const std::vector<unsigned int> indices = {
        0, 1, 2,
        1, 3, 2
    };

    Mesh square(vertices, indices);

    GeometryThing triangle("triangle", &square, "engine/base_shaders/fragment.glsl");

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        triangle.update();
        triangle.render();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return &ge;
}

