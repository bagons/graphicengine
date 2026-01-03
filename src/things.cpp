// std
#include <iostream>
#include <utility>
#include <vector>

// open gl
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

// engine
#include "graphicengine.hpp"




Camera::Camera(const glm::mat4 &projection_matrix) : SpatialThing(false, true){
    projection = projection_matrix;
    view = glm::mat4(1.0);
}

Camera::Camera(float _fov, float _near_plane, float _far_plane) : SpatialThing(false, true) {
    fov = _fov;
    near_plane = _near_plane;
    far_plane = _far_plane;
    projection = glm::perspective(glm::radians(fov), static_cast<float>(ge.window.width) / ge.window.height, near_plane, far_plane);
    view = glm::mat4(1.0);
}

Camera::Camera(float _near_plane, float _far_plane) : SpatialThing(false, true) {
    near_plane = _near_plane;
    far_plane = _far_plane;
    projection = glm::ortho(0.0f, static_cast<float>(ge.window.width), 0.0f, static_cast<float>(ge.window.height), near_plane, far_plane);
    view = glm::mat4(1.0);
}

void Camera::change_resolution(const int width, const int height) {
    if (fov == 0) {
        projection = glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height), near_plane, far_plane);
    } else {
        projection = glm::perspective(glm::radians(fov), static_cast<float>(width) / height, near_plane, far_plane);
    }
}

void Camera::transform_to_view_matrix() {
    // reverse rotation
    transform.rotation.euler_2_quat();
    transform.rotation.i *= -1;
    transform.rotation.j *= -1;
    transform.rotation.k *= -1;

    // reverse position
    transform.position.x *= -1;
    transform.position.y *= -1;
    transform.position.z *= -1;

    view = transform.scale.get_transformation_matrix() * transform.rotation.get_transformation_matrix() * transform.position.get_transformation_matrix();

    // reverse rotation back
    transform.rotation.i *= -1;
    transform.rotation.j *= -1;
    transform.rotation.k *= -1;

    // reverse position back
    transform.position.x *= -1;
    transform.position.y *= -1;
    transform.position.z *= -1;
}


//
// THINGS:
//

Thing::Thing (const bool is_renderable, const bool is_updatable) {
    renderable = is_renderable;
    updatable = is_updatable;
};

Thing::~Thing() {};

void Thing::render() {};
void Thing::update() {};


SpatialThing::SpatialThing(const bool is_renderable, const bool is_updatable) : Thing(is_renderable, is_updatable) {
    transform = Transform{};
}



MeshThing::MeshThing (std::shared_ptr<Mesh> _mesh, std::shared_ptr<Material> _material) : SpatialThing(true, true) {
    // mesh setup
    mesh = std::move(_mesh);
    material = std::move(_material);

    vs_uniform_transform_loc = glGetUniformLocation(material->shader_program.id, "transform");
}

void MeshThing::update() {

}


void MeshThing::render() {
    glm::mat4 model = transform.position.get_transformation_matrix() * transform.rotation.get_transformation_matrix() * transform.scale.get_transformation_matrix();

    glUniformMatrix4fv(vs_uniform_transform_loc, 1, GL_FALSE, &model[0][0]);
    glBindVertexArray(mesh->vertex_array_object);
    glDrawElements(GL_TRIANGLES, mesh->vertex_count, GL_UNSIGNED_INT, 0);
}


ModelThing::ModelThing(std::shared_ptr<Model> _model, std::vector<std::shared_ptr<Material>> _materials) : SpatialThing(false, true) {
    model = std::move(_model);
    materials = std::move(_materials);
    const int model_geref_id = ge.next_thing_id - 1;
    for (size_t i = 0; i < model->meshes.size(); i++) {
        std::shared_ptr<Material> mat;
        // if no custom material load model material
        if (i >= materials.size() or materials[i] == nullptr) {
            mat = model->materials[i];
        } // else load custom material
        else {
            mat = materials[i];
        }

        // if some of the materials are nullptr -> equivalent to the base material
        if (mat == nullptr)
            mat = ge.shaders.get_base_material(model->has_uvs, model->has_normals);

        // spawn slave
        auto model_slave = ge.add<ModelSlaveThing>(model->meshes[i], mat, geRef<ModelThing>(model_geref_id, &ge));
    }
}


void ModelSlaveThing::render() {
    // copy manager position (the slave part)
    transform = manager->transform;

    // standard MeshThing render
    glm::mat4 model = transform.position.get_transformation_matrix() * transform.rotation.get_transformation_matrix() * transform.scale.get_transformation_matrix();

    glUniformMatrix4fv(vs_uniform_transform_loc, 1, GL_FALSE, &model[0][0]);
    glBindVertexArray(mesh->vertex_array_object);
    glDrawElements(GL_TRIANGLES, mesh->vertex_count, GL_UNSIGNED_INT, 0);
}

PointLightThing::PointLightThing(const glm::vec4 _light_data, const bool is_updatable) : SpatialThing(false, is_updatable) {
    light_data = _light_data;
}