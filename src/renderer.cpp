#include "renderer.hpp"
#include "shaders.hpp"
#include "graphicengine.hpp"
#include "gtc/type_ptr.inl"

ForwardOpaque3DPass::ForwardOpaque3DPass(const geRef<Camera> _camera) {
    camera = _camera;
}


void ForwardOpaque3DPass::render() {
    camera->transform_to_view_matrix();

    ge.lights.update(camera->transform.position);

    // update Camera Data Uniform Buffer
    glBindBuffer(GL_UNIFORM_BUFFER, ge.camera_matrix_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(camera->projection));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(camera->view));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    unsigned int current_sp = -1;
    uint64_t current_mat_id = -1;
    //ge.base_material->shader_program.use();
    for (const auto& [mat, id] : ge.thing_ids_by_shader_program) {
        // skip not renderable entities
        if (!ge.get_thing(id)->visible)
            continue;
        // switch shader program if need be
        if (mat->shader_program.get_id() != current_sp) {
            current_sp = mat->shader_program.get_id();
            mat->shader_program.use();
        }
        // update uniform values only if mat id changes, which means we can repeat uniform setting, but only when 2 different material have the same values
        if (mat->id != current_mat_id) {
            current_mat_id = mat->id;
            mat->set_uniform_values();
        }

        // render thing
        ge.get_thing(id)->render();
    }
}

void ForwardOpaque3DPass::change_resolution(const int width, const int height) {
    camera->change_resolution(width, height);
}
