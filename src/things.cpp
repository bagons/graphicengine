// std
#include <utility>
#include <vector>

// open gl
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

// engine
#include <iostream>

#include "graphicengine.hpp"



Camera::Camera(const glm::mat4 &projection_matrix) {
    projection = projection_matrix;
    view = glm::mat4(1.0);
}

Camera::Camera(float _fov, float _near_plane, float _far_plane) {
    fov = _fov;
    near_plane = _near_plane;
    far_plane = _far_plane;
    projection = glm::perspective(glm::radians(fov), static_cast<float>(ge.window.width) / static_cast<float>(ge.window.height), near_plane, far_plane);
    view = glm::mat4(1.0);
}

Camera::Camera(float _near_plane, float _far_plane) {
    near_plane = _near_plane;
    far_plane = _far_plane;
    projection = glm::ortho(0.0f, static_cast<float>(ge.window.width), 0.0f, static_cast<float>(ge.window.height), near_plane, far_plane);
    view = glm::mat4(1.0);
}

float Camera::get_fov() const {
    return fov;
}

float Camera::get_near_plane() const {
    return near_plane;
}

float Camera::get_far_plane() const {
    return far_plane;
}


void Camera::change_resolution(const int width, const int height) {
    if (fov == 0) {
        projection = glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height), near_plane, far_plane);
    } else {
        projection = glm::perspective(glm::radians(fov), static_cast<float>(width) / static_cast<float>(height), near_plane, far_plane);
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

void Thing::render() {};
void Thing::update() {};
void Thing::on_remove() {};


SpatialThing::SpatialThing() {
    transform = Transform{};
}



MeshThing::MeshThing (std::shared_ptr<Mesh> _mesh, std::shared_ptr<Material> _material, unsigned int _render_layer) {
    render_layer = _render_layer;
    // mesh setup
    mesh = std::move(_mesh);
    material = std::move(_material);

    vs_uniform_transform_loc = glGetUniformLocation(material->shader_program.get_id(), "transform");
}

std::shared_ptr<Mesh> MeshThing::get_mesh() {
    return mesh;
}

std::shared_ptr<Material> MeshThing::get_material() {
    return material;
}


void MeshThing::render() {
    glm::mat4 model = transform.position.get_transformation_matrix() * transform.rotation.get_transformation_matrix() * transform.scale.get_transformation_matrix();

    glUniformMatrix4fv(vs_uniform_transform_loc, 1, GL_FALSE, &model[0][0]);
    glBindVertexArray(mesh->get_vertex_array_object());
    glDrawElements(GL_TRIANGLES, mesh->get_vertex_count(), GL_UNSIGNED_INT, nullptr);
}


ModelThing::ModelThing(std::shared_ptr<Model> _model, std::vector<std::shared_ptr<Material>> _materials, unsigned int _render_layer) {
    model = std::move(_model);
    materials = std::move(_materials);
    const unsigned int model_geref_id = ge.get_last_used_geRef_id();
    std::cout << "model thing id: " << model_geref_id << std::endl;
    for (size_t i = 0; i < model->get_mesh_count(); i++) {
        std::shared_ptr<Material> mat;
        // if no custom material load model material
        if (i >= materials.size() or materials[i] == nullptr) {
            mat = model->get_material(i);
        } // else load custom material
        else {
            mat = materials[i];
        }

        // if some of the materials are nullptr -> equivalent to the base material
        if (mat == nullptr)
            mat = ge.shaders.get_base_material(model->get_has_uvs(), model->get_has_normals());

        // spawn slave
        auto slave = ge.add<ModelSlaveThing>(model->get_mesh(i), mat, geRef<ModelThing>(model_geref_id, &ge));
        slave_ids.push_back(slave.id);
        slave->render_layer = _render_layer;
    }
}

std::shared_ptr<Model> ModelThing::get_model() {
    return model;
}

std::shared_ptr<Material> ModelThing::get_material(const size_t index) {
    return materials[index];
}

void ModelThing::on_remove() {
    for (const auto slave : slave_ids) {
        ge.remove_thing(slave);
    }
}


ModelSlaveThing::ModelSlaveThing(std::shared_ptr<Mesh> _mesh, std::shared_ptr<Material> _material, const geRef<ModelThing> _manager):
MeshThing(std::move(_mesh), std::move(_material)) {
    manager = _manager;
};



void ModelSlaveThing::render() {
    // copy manager position (the slave part)
    transform = manager->transform;

    // standard MeshThing render
    glm::mat4 model = transform.position.get_transformation_matrix() * transform.rotation.get_transformation_matrix() * transform.scale.get_transformation_matrix();

    glUniformMatrix4fv(vs_uniform_transform_loc, 1, GL_FALSE, &model[0][0]);
    glBindVertexArray(mesh->get_vertex_array_object());
    glDrawElements(GL_TRIANGLES, mesh->get_vertex_count(), GL_UNSIGNED_INT, nullptr);
}