#include "meshes.hpp"
#include <glad/glad.h>

Mesh::Mesh(std::vector<float> vertices, std::vector<unsigned int> indices) {
    glGenBuffers(1, &vertex_buffer_object);
    glGenBuffers(1, &element_buffer_object);

    glGenVertexArrays(1, &vertex_array_object);
    glBindVertexArray(vertex_array_object);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_object);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    vertex_count = indices.size();

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
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
        cube = new Mesh{CUBE_VERTICES, CUBE_INDICES};
    }
    return cube;
}