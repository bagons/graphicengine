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
    /// @param has_tangents data contains tangent data
    /// @param has_vertex_colors data contains vertex colors
    void load_mesh_to_gpu(const std::vector<float>* vertex_data, const std::vector<unsigned int>* indices, bool has_uvs, bool has_normals, bool has_tangents, bool has_vertex_colors = false);
    bool has_uvs = false;
    bool has_normals = false;
    bool has_tangents = false;

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
    Mesh(const std::vector<float>* vertices, const std::vector<unsigned int>* indices, bool has_uvs = true, bool has_normals = true, bool has_tangents = false, bool has_vertex_colors = false);
    /// Allocates Mesh to GPU from .obj file
    /// @param file_path path to a .obj file relative from .exe
    /// @param generate_tangents if tangents need to be generated and added to mesh data, say yes if you plan on using HEIGHT or NORMAL MAPS in FRAGMENT SHADER.
    explicit Mesh(const char* file_path, bool generate_tangents = false);

    /// getter for read-only has_uvs parameter
    [[nodiscard]] bool does_have_uvs() const;
    /// getter for read-only does_have_normals parameter
    [[nodiscard]] bool does_have_normals() const;

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
    /// @brief 3 ways to deal with Tangents when parsing a model
    /// AUTO_GENERATE - will handle decisions for you, if tangents are needed the will generated otherwise not
    /// FORCE_GENERATE_ALL - will generate tangents for all, their usage is optional
    /// FORCE_NO_GENERATION - will not generate any tangents, but normal and bump maps will be ignored
    enum TangentAction {
        AUTO_GENERATE,
        FORCE_GENERATE_ALL,
        FORCE_NO_GENERATION
    };
    /// a material by index getter, because list of pointers is read only
    /// @param index index
    [[nodiscard]] std::shared_ptr<Material> get_material(size_t index) const;
    /// a mesh by index getter, because list of pointers is read only
    /// @param index index
    [[nodiscard]] std::shared_ptr<Mesh> get_mesh(size_t index) const;
    /// mesh count getter, but also material count getter, because both have the same value
    [[nodiscard]] size_t get_mesh_count() const;

    /// getter for read-only has_uvs
    bool get_has_uvs() const;

    /// getter for read-only has_normals
    bool get_has_normals() const;

    explicit Model(const char* file_path, const TangentAction& action = AUTO_GENERATE);
};

/// A default Mesh houser
/// @note loads default meshes to GPU, it's a few bytes, but if really don't want them loaded set pointers to nullptr and don't use them anywhere, there are going to get cleared
class Meshes {
    std::shared_ptr<Mesh> plane;
    std::shared_ptr<Mesh> sphere;
    std::shared_ptr<Mesh> cube;

    std::shared_ptr<Mesh> tangent_plane;
    std::shared_ptr<Mesh> tangent_sphere;
    std::shared_ptr<Mesh> tangent_cube;
public:
    /// default plane
    /// @param with_tangents if the Mesh has tangent data, meaning that normal or bump maps can be applied
    [[nodiscard]] std::shared_ptr<Mesh> get_plane(bool with_tangents = false) const;
    /// default cube
    /// @param with_tangents if the Mesh has tangent data, meaning that normal or bump maps can be applied
    [[nodiscard]] std::shared_ptr<Mesh> get_cube(bool with_tangents = false) const;
    /// default sphere
    /// @param with_tangents if the Mesh has tangent data, meaning that normal or bump maps can be applied
    [[nodiscard]] std::shared_ptr<Mesh> get_sphere(bool with_tangents = false) const;

    /// load default Meshes to GPU
    void load_base_meshes();

    /// unload default Meshes form GPU
    void unload_base_meshes();

    Meshes() = default;
};
#endif //MESHES_HPP
