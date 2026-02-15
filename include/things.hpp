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
/// @ingroup Things
class Thing {
public:
    /// Bool whether entity is going to be rendered
    bool renderable;
    /// Bool whether entity is going to be updated
    bool updatable;

    Thing (bool is_renderable, bool is_updatable);

    /// Called by RenderLayers
    virtual void render();
    /// Called by Engine.update() method
    virtual void update();

    virtual ~Thing();
};

/// Entity with a Transform
/// @ingroup Things
class SpatialThing : public Thing {
public:
    /// describes the position, rotation, and scale
    Transform transform;
    SpatialThing(bool is_renderable, bool is_updatable);
};

/// Represents camera, acts as a normal entity. Many instances may be spawned. But ForwardRenderer3DLayer takes only one as a param in the render method.
/// @ingroup Things
class Camera : public SpatialThing {
    float near_plane = 0;
    float far_plane = 0;
    float fov = 0;
public:
    /// The VIEW matrix of the camera, same information as the transform.
    /// @warning not updated on a within frame basis, only when transform_to_view_matrix() is called
    glm::mat4 view{};
    glm::mat4 projection{};

    /// Field of view getter, only used for a perspective camera
    [[nodiscard]] float get_fov() const;
    /// Distance of a plane from the camera from which vertices are visible. Meaning lower = camera can see closer objects.
    [[nodiscard]] float get_near_plane() const;
    /// Distance of a plane from the camera from which vertices are no longer visible. Meaning higher = camera can see further objects.
    [[nodiscard]] float get_far_plane() const;

    /// Constructs the camera based on a complete PROJECTION matrix
    /// @param projection_matrix the PROJECTION matrix
    explicit Camera(const glm::mat4 &projection_matrix);
    /// Constructs a perspective camera (PROJECTION matrix gets created in the constructor based on params)
    /// @param _fov Field of view of camera
    /// @param _far_plane
    /// @param _near_plane
    Camera(float _fov, float _far_plane, float _near_plane);
    /// Constructs an orthographic camera (PROJECTION matrix gets created in the constructor based on params)
    /// @param _far_plane
    /// @param _near_plane
    Camera(float _far_plane, float _near_plane);
    /// Updates projection matrix based on resolution to keep the same scale. Called by Window resize event.
    /// @param width width in pixels
    /// @param height height in pixels
    void change_resolution(int width, int height);
    /// transforms the Transform of the camera to the VIEW matrix. Called by the ForwardRenderer3DLayer.render() method.
    void transform_to_view_matrix();
};


/// Spatial entity representing a single mesh with a material
/// @ingroup Things
class MeshThing : public SpatialThing {
protected:
    /// Shared pointer to the mesh
    std::shared_ptr<Mesh> mesh;
    /// Shared pointer to the material
    std::shared_ptr<Material> material;
public:
    /// Uniform location for the transformation matrix in the vertex shader
    int vs_uniform_transform_loc = 0;

    /// read-only Mesh shared pointer getter, may be used for creating a new entity with the same Mesh
    [[nodiscard]] std::shared_ptr<Mesh> get_mesh();
    /// read-only Material shared pointer getter, may be used for creating a new entity with the same Material
    [[nodiscard]] std::shared_ptr<Material> get_material();

    /// Constructs a MeshThing using a Mesh resource and Material resource
    /// @param _mesh the mesh that's going to be rendered
    /// @param _material the material that the mesh is going to be rendered with
    MeshThing (std::shared_ptr<Mesh> _mesh, std::shared_ptr<Material> _material);

    /// Submits the mesh to the GPU for rendering
    void render() override;
};


/// A spatial entity representing a Model (multiple MeshThings). Spawns N ModelSlaveThing, which act as a child of this entity.
/// @ingroup Things
class ModelThing : public SpatialThing {
protected:
    std::shared_ptr<Model> model;
    std::vector<std::shared_ptr<Material>> materials;
public:
    /// read-only Model shared_ptr
    [[nodiscard]] std::shared_ptr<Model> get_model();
    [[nodiscard]] std::shared_ptr<Material> get_material(size_t index);

    /// Constructs a ModelThing
    /// @param _model model resource used
    /// @param _materials list of materials that override model materials. Works on a per-material basis, meaning: [Mat1, nullptr, Mat2, Mat3] -> 1st, 3rd, and 4th overwritten. If the list is shorter: [Mat1, Mat2] the rest is considered as nullptr, thus no override.
    explicit ModelThing(std::shared_ptr<Model> _model, std::vector<std::shared_ptr<Material>> _materials = {});
};


/// Spawned by a ModelThing acts as a child of ModelThing. Not ment for inheriting any further.
/// @ingroup Things
class ModelSlaveThing final : public MeshThing {
public:
    /// Reference to the parent ModelThing that manages this slave
    geRef<ModelThing> manager;

    /// Constructs a MeshThing a links the manager
    /// @param _mesh Forwards parameter to MeshThing constructor
    /// @param _material Forwards parameter to MeshThing constructor
    /// @param _manager Reference to the owner ModelThing
    ModelSlaveThing (std::shared_ptr<Mesh> _mesh, std::shared_ptr<Material> _material, const geRef<ModelThing> _manager) :
    MeshThing(std::move(_mesh), std::move(_material)) {
        manager = _manager;
    };

    /// Inherits transform from manager and submits mesh to GPU for rendering.
    void render() override;
};

#endif //THINGS_H
