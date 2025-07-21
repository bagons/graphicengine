#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "utils.h"
#include <GLFW/glfw3.h>

#include "ext/matrix_clip_space.hpp"
#include "ext/matrix_transform.hpp"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

class Mesh {
    public:
        unsigned int vertex_buffer_object = 0;
        unsigned int vertex_array_object = 0;
        unsigned int element_buffer_object = 0;
        size_t vertex_count = 0;

        Mesh(std::vector<float> vertices, std::vector<unsigned int> indices) {
            glGenBuffers(1, &vertex_buffer_object);
            glGenBuffers(1, &element_buffer_object);

            glGenVertexArrays(1, &vertex_array_object);
            glBindVertexArray(vertex_array_object);

            glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_object);
            std::cout << indices.size() * sizeof(unsigned int) << std::endl;
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
            vertex_count = indices.size();

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
            glEnableVertexAttribArray(0);

            // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);

            std::cout << vertex_buffer_object << std::endl;
            std::cout << element_buffer_object << std::endl;
        }

        ~Mesh() {
            glDeleteVertexArrays(1,  &vertex_array_object);
            glDeleteBuffers(1, &vertex_buffer_object);
            glDeleteBuffers(1, &element_buffer_object);
        }
};

class Thing {
    public:
        std::string name;
        bool renderable;
        bool updatable;

        Thing (const bool is_renderable, const bool is_updatable, std::string object_name) {
            renderable = is_renderable;
            updatable = is_updatable;
            name = std::move(object_name);
        };
};


class SpatialThing : public Thing {
public:
    glm::mat4 transform = glm::mat4(1.0f);
    SpatialThing(const bool is_renderable, const bool is_updatable, std::string object_name) : Thing(is_renderable, is_updatable, std::move(object_name)) {}
};

class Camera : public SpatialThing {
public:
    glm::mat4 projection;
    Camera(std::string object_name, const glm::mat4 &projection_matrix) : SpatialThing(false, false, std::move(object_name)) {
        projection = projection_matrix;
    }
};

class Engine {
    public:
    unsigned int base_vertex_shader;
    Camera *main_camera = nullptr; // temporary

    void load_base_vertex_shader() {
        base_vertex_shader = compile_shader_from_file("engine/base_shaders/vertex.glsl", GL_VERTEX_SHADER);
    }

    ~Engine() {
        glDeleteShader(base_vertex_shader);
    }
};

Engine ge;


class GeometryThing : public SpatialThing {
public:
    unsigned int shader_program = 0;
    unsigned int vs_uniform_transform_loc = 0;
    unsigned int vs_uniform_view_loc = 0;
    Mesh* mesh;

    GeometryThing (GeometryThing &&) = delete;
    GeometryThing (const GeometryThing &) = delete;

    GeometryThing (std::string object_name, Mesh* _mesh, const char* fragment_shader_path) : SpatialThing(true, true, std::move(object_name)) {
        // mesh setup
        mesh = _mesh;

        // setup shaders
        shader_program = glCreateProgram();
        unsigned int fragment_shader_id = compile_shader_from_file(fragment_shader_path, GL_FRAGMENT_SHADER);
        if (!fragment_shader_id) std::cout << "Error creating fragment shader" << std::endl;


        glAttachShader(shader_program, ge.base_vertex_shader);
        glAttachShader(shader_program, fragment_shader_id);
        glLinkProgram(shader_program);

        vs_uniform_transform_loc = glGetUniformLocation(shader_program, "transform");
        vs_uniform_view_loc = glGetUniformLocation(shader_program, "view");

        glUseProgram(shader_program);
        glUniformMatrix4fv(glGetUniformLocation(shader_program, "projection"), 1, GL_FALSE, &ge.main_camera->projection[0][0]);

        glDeleteShader(fragment_shader_id);


    }

    ~GeometryThing () {
        glDeleteProgram(shader_program);
    }

    void update() {
        transform[3][0] = 0.2f * static_cast<float>(glfwGetTime());
        ge.main_camera->transform[3][1] = -0.01f * static_cast<float>(glfwGetTime());
        ge.main_camera->transform[3][2] = -3 - 0.005f * static_cast<float>(glfwGetTime());
    }

    void render() const {
        glUseProgram(shader_program);
        std::cout << vs_uniform_transform_loc << " " << vs_uniform_view_loc << std::endl;
        glUniformMatrix4fv(vs_uniform_transform_loc, 1, GL_FALSE, &transform[0][0]);
        glUniformMatrix4fv(vs_uniform_view_loc, 1, GL_FALSE, &ge.main_camera->transform[0][0]);
        glBindVertexArray(mesh->vertex_array_object);
        glDrawElements(GL_TRIANGLES, mesh->vertex_count, GL_UNSIGNED_INT, 0);
    }
};




Engine* gameengine() {
    /* Initialize the library */
    if (!glfwInit())
        return nullptr;

    /* Create a windowed mode window and its OpenGL context */
    GLFWwindow *window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Hello World", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return nullptr;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    // load glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return nullptr;
    }
    ge.load_base_vertex_shader();

    Camera cam("camera", glm::perspective(glm::radians(45.0f),
    (float) SCREEN_WIDTH / (float) SCREEN_HEIGHT, 0.1f, 100.0f));

    ge.main_camera = &cam;

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

