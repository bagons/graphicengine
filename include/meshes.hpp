#ifndef MESHES_HPP
#define MESHES_HPP

#pragma once
#include <vector>
#include <memory>
#include "shaders.hpp"

/// @ingroup Resources
class Mesh {
void load_mesh_to_gpu(const std::vector<float>* vertex_data, const std::vector<unsigned int>* indices, bool has_uvs, bool has_normals, bool has_vertex_colors = false);
public:
    unsigned int vertex_buffer_object = 0;
    unsigned int vertex_array_object = 0;
    unsigned int element_buffer_object = 0;
    int vertex_count = 0;
    bool has_uvs = false;
    bool has_normals = false;

    Mesh(const std::vector<float>* vertices, const std::vector<unsigned int>* indices, bool has_uvs = true, bool has_normals = true, bool has_vertex_colors = false);
    explicit Mesh(const char* file_path);
    Mesh();

    ~Mesh();
};

/// @ingroup Resources
class Model {
public:
    std::vector<std::shared_ptr<Mesh>> meshes;
    std::vector<std::shared_ptr<Material>> materials;
    bool has_uvs = false;
    bool has_normals = false;
    explicit Model(const char* file_path);
};


class Meshes {
public:
    const std::vector<float> CUBE_VERTEX_DATA = {
        -0.5f, -0.5f, -0.5f,/*texture cords:*/ 0.0, 0.0, 1.0f, 0.0, 0.0,
        0.5f, -0.5f, -0.5f,/*texture cords:*/ 1.0, 0.0, 0.0f, 1.0f, 0.0,
        -0.5f,  0.5f, -0.5f,/*texture cords:*/ 0.0, 1.0, 0.0f, 0.0, 1.0f,
        0.5f, 0.5f, -0.5f,/*texture cords:*/ 1.0, 1.0, 1.0f, 0.0, 0.0,

        -0.5f, -0.5f, 0.5f,/*texture cords:*/ 0.0, 0.0, 0.0f, 1.0f, 0.0,
        0.5f, -0.5f, 0.5f,/*texture cords:*/ 1.0, 0.0, 0.0f, 0.0, 1.0f,
        -0.5f,  0.5f, 0.5f,/*texture cords:*/ 0.0, 1.0, 1.0f, 0.0, 0.0,
        0.5f, 0.5f, 0.5f,/*texture cords:*/ 1.0, 1.0, 0.0f, 1.0f, 0.0
    };

    const std::vector<unsigned int> CUBE_INDICES = {
        0, 1, 2,
        1, 3, 2,
        6, 7, 3,
        3, 2, 6,
        4, 5, 6,
        5, 7, 6
    };

    const std::vector<float> PLANE_VERTEX_DATA = {
        -0.5f, 0.0f, -0.5f, 0.0, 0.0, 0.0, 1.0, 0.0,
        -0.5f, 0.0f, 0.5f, 0.0, 1.0, 0.0, 1.0, 0.0,
        0.5f, 0.0f, 0.5f, 1.0, 1.0, 0.0, 1.0, 0.0,
        0.5f, 0.0f, -0.5f, 1.0, 0.0, 0.0, 1.0, 0.0
    };

    const std::vector<unsigned int> PLANE_INDICES = {0, 1, 2, 0, 2, 3};

    std::shared_ptr<Mesh> plane;
    Meshes();
};
#endif //MESHES_HPP
