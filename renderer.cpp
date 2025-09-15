#include "renderer.hpp"
#include "shaders.hpp"
#include <memory>

#include "gtc/type_ptr.inl"

ForwardRenderer3DLayer::ForwardRenderer3DLayer(const geRef<Camera> camera) {
    cam = camera;
}


void ForwardRenderer3DLayer::render() {
    cam->transform_to_view_matrix();

    // update Camera Data Uniform Buffer
    glBindBuffer(GL_UNIFORM_BUFFER, ge.camera_matrix_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(cam->projection));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(cam->view));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    int current_sp = -1;
    for (const auto& [mat, id] : ge.thing_ids_by_shader_program) {
        // switch shader program if need be
        if (id != current_sp) {
            current_sp = id;
            mat->shader_program->use();
        }

        // for now always set values, even though they could be the same
        mat->set_uniform_values();
        // render thing
        ge.get_thing(id)->render();
    }
}
