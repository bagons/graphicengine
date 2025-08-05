#include "meshes.hpp"
#include <glad/glad.h>
#include <fstream>
#include <iostream>
#include <string>


void Mesh::load_mesh_to_gpu(const std::vector<float>* vertex_data, const std::vector<unsigned int>* indices, const bool has_vertex_colors, const bool has_texture_cords) {
    glGenBuffers(1, &vertex_buffer_object);
    glGenBuffers(1, &element_buffer_object);

    glGenVertexArrays(1, &vertex_array_object);
    glBindVertexArray(vertex_array_object);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, vertex_data->size() * sizeof(float), vertex_data->data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_object);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices->size() * sizeof(unsigned int), indices->data(), GL_STATIC_DRAW);
    vertex_count = indices->size();

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    if (has_vertex_colors) {
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, (6 + 2 * has_texture_cords) * sizeof(float), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    if (has_texture_cords) {
        glVertexAttribPointer(1 + has_vertex_colors, 2, GL_FLOAT, GL_FALSE, (5 + 3 * has_vertex_colors) * sizeof(float), (void*)((3 + 3 * has_vertex_colors)*sizeof(float)));
        glEnableVertexAttribArray(1 + has_vertex_colors);
    }

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
};


Mesh::Mesh(const std::vector<float>* vertex_data, const std::vector<unsigned int>* indices, const bool has_vertex_colors, const bool has_texture_cords) {
    load_mesh_to_gpu(vertex_data, indices, has_vertex_colors, has_texture_cords);
}

Mesh::Mesh(){
    vertex_buffer_object = -1;
    vertex_array_object = -1;
    element_buffer_object = -1;
}

Mesh::~Mesh() {
    glDeleteVertexArrays(1,  &vertex_array_object);
    glDeleteBuffers(1, &vertex_buffer_object);
    glDeleteBuffers(1, &element_buffer_object);
}

Meshes::Meshes() {

}

Meshes::~Meshes() {
    if (cube != nullptr) {
        delete cube;
    }
}

Mesh* Meshes::get_cube() {
    if (cube == nullptr) {
        cube = new Mesh{&CUBE_VERTEX_DATA, &CUBE_INDICES, true, true};
    }
    return cube;
}