#ifndef MESHES_HPP
#define MESHES_HPP

#pragma once
#include <vector>
#include <memory>

class Mesh {
void load_mesh_to_gpu(const std::vector<float>* vertex_data, const std::vector<unsigned int>* indices, bool has_texture_cords, bool has_normals, bool has_vertex_colors);
public:
    unsigned int vertex_buffer_object = 0;
    unsigned int vertex_array_object = 0;
    unsigned int element_buffer_object = 0;
    int vertex_count = 0;

    Mesh(const std::vector<float>* vertices, const std::vector<unsigned int>* indices, bool has_texture_cords = true, bool has_normals = true, bool has_vertex_colors = false);
    explicit Mesh(const char* file_path, bool has_texture_cords = true, bool has_normals = true, bool has_vertex_colors = false);
    Mesh();

    ~Mesh();
};

void parse_obj_file(const char* file_path, bool merge_all_groups, std::vector<float> (&vertex_data_vec)[3], std::vector<std::vector<size_t>>& vertex_groups, bool& has_normals, bool& has_texture_cords);
void construct_mesh_data_from_parsed_obj_data(const std::vector<float> (&vertex_data_vec)[3], const std::vector<size_t>& vertex_triplets, bool has_normals, bool has_texture_cords, std::vector<float>& out_vertex_data, std::vector<unsigned int>& out_indices);


class Model {
public:
    std::vector<std::shared_ptr<Mesh>> meshes;
    explicit Model(const char* file_path, bool has_texture_cords = true, bool has_normals = true, bool has_vertex_colors = false);
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
