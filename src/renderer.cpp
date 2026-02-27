#include "renderer.hpp"
#include "shaders.hpp"
#include "graphicengine.hpp"
#include "gtc/type_ptr.inl"

ColorPass::ColorPass(Color color) : color(color) {

};

void ColorPass::render() {
    glClearColor(color.r, color.g, color.b, color.a);
}

ForwardOpaque3DPass::ForwardOpaque3DPass(const geRef<Camera> _camera, const unsigned int _render_layer) {
    render_layer = _render_layer;
    camera = _camera;
}


void ForwardOpaque3DPass::render() {
    if (ge.auto_clear_screen and (!ge.was_color_buffer_cleared() or !ge.was_depth_buffer_cleared())) {
        ge.clear_framebuffers();
    }

    camera->transform_to_view_matrix();

    ge.lights.update(camera->transform.position);

    // update Camera Data Uniform Buffer
    glBindBuffer(GL_UNIFORM_BUFFER, ge.camera_matrix_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(camera->projection));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(camera->view));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    unsigned int current_sp = -1;
    uint64_t current_mat_id = -1;

    for (const auto& [mat, id] : ge.thing_ids_by_shader_program) {
        // skip not renderable entities || or || an entity that does bitwise match by render layer
        if (!ge.get_thing(id)->visible or !(ge.get_thing(id)->render_layer & render_layer))
            continue;
        // switch shader program if need be
        if (mat->get_shader_program_id() != current_sp) {
            current_sp = mat->get_shader_program_id();
            mat->get_shader_program().use();
        }
        // update uniform values only if mat id changes, which means we can repeat uniform setting, but only when 2 different material have the same values
        if (mat->get_id() != current_mat_id) {
            current_mat_id = mat->get_id();
            mat->apply_uniform_values();
        }

        // render thing
        ge.get_thing(id)->render();
    }
}

void ForwardOpaque3DPass::change_resolution(const int width, const int height) {
    camera->change_resolution(width, height);
}