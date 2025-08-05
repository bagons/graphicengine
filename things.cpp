// std
#include <string>
#include <iostream>
#include <vector>

// open gl
#include <glad/glad.h>
#include <glm/glm.hpp>

// engine
#include "gameengine.hpp"




Camera::Camera(const glm::mat4 &projection_matrix) {
    projection = projection_matrix;
    view = glm::mat4(1.0);
}


//
// THINGS:
//

Thing::Thing (const bool is_renderable, const bool is_updatable, std::string object_name) {
    renderable = is_renderable;
    updatable = is_updatable;
    name = std::move(object_name);
};

Thing::~Thing() {};

void Thing::render(Camera *) {};
void Thing::update() {};


SpatialThing::SpatialThing(const bool is_renderable, const bool is_updatable, std::string object_name) : Thing(is_renderable, is_updatable, std::move(object_name)) {
    transform = glm::mat4(1.0);
}



GeometryThing::GeometryThing (std::string object_name, Mesh* _mesh, const char* fragment_shader_path) : SpatialThing(true, true, std::move(object_name)) {
    // mesh setup
    mesh = _mesh;

    // setup shaders
    shader_program = glCreateProgram();
    unsigned int fragment_shader_id = compile_shader_from_file(fragment_shader_path, GL_FRAGMENT_SHADER);
    if (!fragment_shader_id) std::cout << "Error creating fragment shader" << std::endl;


    glAttachShader(shader_program, ge.base_vertex_shader);
    glAttachShader(shader_program, fragment_shader_id);
    glLinkProgram(shader_program);

    vs_uniform_projection_loc = glGetUniformLocation(shader_program, "projection");
    vs_uniform_transform_loc = glGetUniformLocation(shader_program, "transform");
    vs_uniform_view_loc = glGetUniformLocation(shader_program, "view");



    glDeleteShader(fragment_shader_id);
}

GeometryThing::~GeometryThing () {
    glDeleteProgram(shader_program);
}

void GeometryThing::update() {

}


void GeometryThing::render(Camera * from_camera) {
    glUseProgram(shader_program);

    glUniformMatrix4fv(vs_uniform_transform_loc, 1, GL_FALSE, &transform[0][0]);
    glUniformMatrix4fv(vs_uniform_view_loc, 1, GL_FALSE, &from_camera->view[0][0]);
    glUniformMatrix4fv(vs_uniform_projection_loc, 1, GL_FALSE, &from_camera->projection[0][0]);

    glBindVertexArray(mesh->vertex_array_object);
    glDrawElements(GL_TRIANGLES, mesh->vertex_count, GL_UNSIGNED_INT, 0);
}