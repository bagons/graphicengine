#ifndef MESHES_HPP
#define MESHES_HPP

#pragma once
#include <vector>
#include <memory>
#include "shaders.hpp"


/// @brief Represents a mesh resource on the GPU.
/// Is allocated on the GPU on creation and is deallocated on deconstruction from the GPU.
/// @ingroup Resources
class Mesh {
    /// loads mesh data to the GPU and creates Buffers
    /// @param vertex_data list of floats containing all the vertice data by N float
    /// @param indices triangle definition using indexes that reference vertex_data
    /// @param has_uvs data contains uvs
    /// @param has_normals data contains normals
    /// @param has_vertex_colors data contains vertex colors
    void load_mesh_to_gpu(const std::vector<float>* vertex_data, const std::vector<unsigned int>* indices, bool has_uvs, bool has_normals, bool has_vertex_colors = false);
    bool has_uvs = false;
    bool has_normals = false;

    unsigned int vertex_buffer_object = 0;
    unsigned int vertex_array_object = 0;
    unsigned int element_buffer_object = 0;
    /// amount of vertices in mesh
    int vertex_count = 0;
public:
    /// getter for read-only vertex_buffer_object variable
    [[nodiscard]] unsigned int get_vertex_array_object() const;
    /// getter for read-only vertex count variable
    [[nodiscard]] int get_vertex_count() const;

    /// Allocates Mesh to GPU based on mesh data
    /// @param vertices list of floats containing all the vertice data by N float
    /// @param indices triangle definition using indexes that reference vertex_data
    /// @param has_uvs has uvs
    /// @param has_normals has normals
    /// @param has_vertex_colors has vertex colors
    Mesh(const std::vector<float>* vertices, const std::vector<unsigned int>* indices, bool has_uvs = true, bool has_normals = true, bool has_vertex_colors = false);
    /// Allocates Mesh to GPU from .obj file
    /// @param file_path path to a .obj file relative from .exe
    explicit Mesh(const char* file_path);
    /// create empty wrapper
    Mesh();

    /// Deallocates Mesh from the GPU
    /// @warning do not do on a thread different from the main
    ~Mesh();
};

/// Model contain N meshes and N materials, represents a multicolored 3D model.
/// @ingroup Resources
class Model {
    std::vector<std::shared_ptr<Mesh>> meshes;
    std::vector<std::shared_ptr<Material>> materials;

    bool has_uvs = false;
    bool has_normals = false;
public:
    /// a material by index getter, because list of pointers is read only
    /// @param index index
    std::shared_ptr<Material> get_material(size_t index) const;
    /// a mesh by index getter, because list of pointers is read only
    /// @param index index
    std::shared_ptr<Mesh> get_mesh(size_t index) const;
    /// mesh count getter, but also material count getter, because both have the same value
    size_t get_mesh_count() const;

    /// getter for read-only has_uvs
    bool get_has_uvs() const;

    /// getter for read-only has_normals
    bool get_has_normals() const;

    explicit Model(const char* file_path);
};

/// A default Mesh houser
/// @note loads default meshes to GPU, it's a few bytes, but if really don't want them loaded set pointers to nullptr and don't use them anywhere, there are going to get cleared
class Meshes {
public:
    /// vertex data for a UV, NORMAL cube
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

    /// indecy data for a cube
    const std::vector<unsigned int> CUBE_INDICES = {
        0, 1, 2,
        1, 3, 2,
        6, 7, 3,
        3, 2, 6,
        4, 5, 6,
        5, 7, 6,
        7, 5, 1,
        1, 3, 7,
        6, 4, 0,
        0, 2, 6,
        4, 5, 1,
        1, 0, 4
    };

    /// vertex data for a UV, NORMAL plane
    const std::vector<float> PLANE_VERTEX_DATA = {
        -0.5f, 0.0f, -0.5f, 0.0, 0.0, 0.0, 1.0, 0.0,
        -0.5f, 0.0f, 0.5f, 0.0, 1.0, 0.0, 1.0, 0.0,
        0.5f, 0.0f, 0.5f, 1.0, 1.0, 0.0, 1.0, 0.0,
        0.5f, 0.0f, -0.5f, 1.0, 0.0, 0.0, 1.0, 0.0
    };

    /// indecy data for a plane
    const std::vector<unsigned int> PLANE_INDICES = {0, 1, 2, 0, 2, 3};

    /// default plane
    std::shared_ptr<Mesh> plane;
    /// default cube
    std::shared_ptr<Mesh> cube;

    /// load default Meshes to GPU
    void load_base_meshes();

    /// unload default Meshes form GPU
    void unload_base_meshes();

    Meshes() = default;
};
#endif //MESHES_HPP
