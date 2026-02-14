#ifndef THINGS_H
#define THINGS_H

#include <glm/glm.hpp>
#include <utility>
#include <memory>
#include "coordinates.h"
#include "shaders.hpp"
#include "meshes.hpp"
#include "gereferences.hpp"

/// Root entity class
class Thing {
public:
    ///
    bool renderable;
    bool updatable;

    Thing (bool is_renderable, bool is_updatable);

    virtual void render();
    virtual void update();

    virtual ~Thing();
};

/// Entity with a Transform
class SpatialThing : public Thing {
public:
    /// describes the position, rotation, and scale
    Transform transform;
    SpatialThing(bool is_renderable, bool is_updatable);
};

/// Represents camera
class Camera : public SpatialThing {
    float near_plane = 0;
    float far_plane = 0;
    float fov = 0;
public:
    /// The VIEW matrix of the camera, same information as the transform.
    /// @warning not updated on a within frame basis, only when transform_to_view_matrix() is called
    glm::mat4 view{};
    glm::mat4 projection{};

    [[nodiscard]] float get_fov() const;
    [[nodiscard]] float get_near_plane() const;
    [[nodiscard]] float get_far_plane() const;
    explicit Camera(const glm::mat4 &projection_matrix);
    explicit Camera(float _fov, float _far_plane, float _near_plane);
    explicit Camera(float _far_plane, float _near_plane);
    void change_resolution(int width, int height);
    void transform_to_view_matrix();
};


class MeshThing : public SpatialThing {
protected:
    /// Shared pointer to the mesh
    std::shared_ptr<Mesh> mesh;
    /// Shared pointer to the material
    std::shared_ptr<Material> material;
public:
    int vs_uniform_transform_loc = 0;

    [[nodiscard]] std::shared_ptr<Mesh> get_mesh();
    [[nodiscard]] std::shared_ptr<Material> get_material();
    MeshThing (std::shared_ptr<Mesh> _mesh, std::shared_ptr<Material> _material);

    /// Submits the mesh to the GPU for rendering
    void render() override;
};


class ModelThing : public SpatialThing {
protected:
    std::shared_ptr<Model> model;
    std::vector<std::shared_ptr<Material>> materials;
public:
    [[nodiscard]] std::shared_ptr<Model> get_model();
    [[nodiscard]] std::shared_ptr<Material> get_material(size_t index);
    explicit ModelThing(std::shared_ptr<Model> _model, std::vector<std::shared_ptr<Material>> _materials = {});
};


class ModelSlaveThing : public MeshThing {
class ModelSlaveThing final : public MeshThing {
public:
    geRef<ModelThing> manager;
    ModelSlaveThing (std::shared_ptr<Mesh> _mesh, std::shared_ptr<Material> _material, const geRef<ModelThing> _manager) : MeshThing(std::move(_mesh), std::move(_material)) {
        manager = _manager;
    };
    void render() override;
};

#endif //THINGS_H
