#ifndef MESHES_HPP
#define MESHES_HPP

#include <vector>

class Mesh {
public:
    unsigned int vertex_buffer_object;
    unsigned int vertex_array_object;
    unsigned int element_buffer_object;
    size_t vertex_count = 0;

    Mesh(std::vector<float> vertices, std::vector<unsigned int> indices);
    Mesh();

    ~Mesh();
};

class Meshes {
public:
    const std::vector<float> CUBE_VERTICES = {
        -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        0.5f, 0.5f, -0.5f,

        -0.5f, -0.5f, 0.5f,
        0.5f, -0.5f, 0.5f,
        -0.5f,  0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
    };

    const std::vector<unsigned int> CUBE_INDICES = {
        0, 1, 2,
        1, 3, 2,
        6, 7, 3,
        3, 2, 6,
        4, 5, 6,
        5, 7, 6
    };

    Mesh* cube;
    Meshes();
    Mesh* get_cube();
    ~Meshes();
};

#endif //MESHES_HPP
